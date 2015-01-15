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

#include <utils.h>
#include <application.h>

using namespace hfsmexec;

/*
 * Downloader
 */
const Logger* Downloader::logger = Logger::getLogger(LOGGER_UTILS);

Downloader::Downloader() :
    reply(NULL)
{
    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

Downloader::~Downloader()
{

}

void Downloader::download(const QString& url, bool block)
{
    logger->info(QString("load file %1").arg(url));

    QUrl u;
    //remote file
    if (url.contains("://"))
    {
        u = QUrl::fromUserInput(url);
    }
    //local file
    else
    {
        u = QUrl::fromLocalFile(url);
    }

    reply = manager.get(QNetworkRequest(u));

    if (block)
    {
        QCoreApplication* qtApplication = Application::getInstance()->getQtApplication();
        do
        {
            qtApplication->processEvents();
        }
        while (!reply->isFinished());
    }
}

const QNetworkReply::NetworkError& Downloader::getError() const
{
    return error;
}

const QString& Downloader::getErrorMessage() const
{
    return errorMessage;
}

const QByteArray& Downloader::getData() const
{
    return data;
}

void Downloader::downloadFinished(QNetworkReply* reply)
{
    data = reply->readAll();
    error = reply->error();

    reply->deleteLater();

    if (error != QNetworkReply::NoError)
    {
        logger->warning(QString("couldn't load file: %1").arg(reply->errorString()));
    }
    else
    {
        logger->info("load file finished");
    }

    emit finished(error, errorMessage, data);
}