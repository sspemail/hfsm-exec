/*
 *  Copyright (C) 2014 Marcel Lehwald
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <statemachine_impl.h>
#include <decoder_impl.h>
#include <parameter_server.h>
#include <api.h>
#include <plugins.h>

#include <QCoreApplication>

using namespace hfsmexec;

int main(int argc, char **argv)
{
    QCoreApplication qtApplication(argc, argv);

    //WebApiTest;
    //new StateMachineTest;
    //XmlDecoderFactoryTest f;
    InvokeStatePluginLoaderTest t;
    //ParameterServerTest p;

    return 0; //TODO
    return qtApplication.exec();
}
