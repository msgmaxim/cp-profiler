/*  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "receiverthread.hh"
#include "globalhelper.hh"
#include "solver_tree_dialog.hh"
#include "message.pb.hh"

#include <QTcpSocket>

// This is a bit wrong.  We have both a separate thread and
// asynchronous reading from the socket.  One or the other would
// suffice.

ReceiverThread::ReceiverThread(int socketDescriptor, Execution* execution, QObject* parent)
    : QThread(parent),
      socketDescriptor(socketDescriptor),
      execution(execution)
{
    connect(this, SIGNAL(startReceiving()), execution, SIGNAL(startReceiving()));
    connect(this, SIGNAL(doneReceiving()), execution, SIGNAL(doneReceiving()));
}


void
ReceiverThread::run(void) {
    tcpSocket = new QTcpSocket;
    if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
        std::cerr << "something went wrong setting the socket descriptor\n";
        abort();
    }

    ReceiverWorker* worker = new ReceiverWorker(tcpSocket, execution);
    connect(tcpSocket, SIGNAL(readyRead()), worker, SLOT(doRead()));

    connect(worker, SIGNAL(startReceiving(void)), this, SIGNAL(startReceiving(void)));
    connect(worker, SIGNAL(doneReceiving(void)), this, SLOT(quit(void)));
    connect(worker, SIGNAL(doneReceiving(void)), this, SIGNAL(doneReceiving(void)));

    std::cerr << "Receiver thread " << this << " running event loop\n";
    exec();
    std::cerr << "Receiver thread " << this << " terminating\n";
}

// This function is called whenever there is new data available to be
// read on the socket.
void
ReceiverWorker::doRead()
{
    while (tcpSocket->bytesAvailable() > 0) {
        // Read all data on the socket into a local buffer.
        buffer.append(tcpSocket->readAll());

        // While we have enough data in the buffer to process the next
        // header or body
        while ((size == 0 && buffer.size() >= 4) || (size > 0 && buffer.size() >= size)) {
            // Read the header (which contains the size of the body)
            if (size == 0 && buffer.size() >= 4) {
                size = ArrayToInt(buffer.mid(0,4));
                buffer.remove(0,4);
            }
            // Read the body
            if (size > 0 && buffer.size() >= size) {
                QByteArray data = buffer.mid(0,size);
                buffer.remove(0, size);
                int msgsize = size;
                size = 0;

                message::Node msg1;
                msg1.ParseFromArray(data, msgsize);

                std::cerr << "message type: " << msg1.type() << "\n";

                switch (msg1.type()) {
                case message::Node::NODE:
                    execution->handleNewNode(msg1);
                    break;
                case message::Node::START: /// TODO: start sending should have model name
                    qDebug() << "START RECEIVING";
                    
                    if (msg1.restart_id() != -1 && msg1.restart_id() != 0) {
                        qDebug() << ">>> restart and continue";
                        break;
                    }

                    emit startReceiving();

                    break;
                case message::Node::DONE:
                    qDebug() << "received DONE SENDING";
                    emit doneReceiving();
                    break;
                }
            }
        }
    }
}
