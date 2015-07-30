/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "globalhelper.hh"
#include <iostream>


GlobalParser* GlobalParser::_self = nullptr;


QCommandLineParser GlobalParser::clParser{};
QCommandLineOption GlobalParser
      ::helpOption{clParser.addHelpOption()};

QCommandLineOption GlobalParser
      ::versionOption{clParser.addVersionOption()};

QCommandLineOption GlobalParser
      ::testOption{"test", "Terminates right after getting DONE RECEIVING"};

QCommandLineOption GlobalParser
      ::portOption{{"p", "port"}, "Send nodes via port <port>.", "port"};


GlobalParser::GlobalParser()
{

  if (_self) {
    std::cerr << "Can't have two of GlobalParser, terminate\n";
    abort();
  }
  _self = this;

  portOption.setDefaultValue("6565");

  clParser.addOption(testOption);
  clParser.addOption(portOption);


}

bool
GlobalParser::isSet(const QCommandLineOption& opt) {
  return _self->clParser.isSet(opt);
}

void
GlobalParser::process(const QCoreApplication & app) {
  _self->clParser.process(app);
}

QString
GlobalParser::value(const QCommandLineOption& opt) {
  return _self->clParser.value(opt);
}

GlobalParser::~GlobalParser() {

}