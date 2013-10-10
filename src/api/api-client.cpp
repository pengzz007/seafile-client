#include <QUrl>
#include <QtNetwork>

#include "api-client.h"

namespace {

const char *kContentTypeForm = "application/x-www-form-urlencoded";
const char *kAuthHeader = "Authorization";

} // namespace

QNetworkAccessManager* SeafileApiClient::na_mgr_ = NULL;

SeafileApiClient::SeafileApiClient()
{
    if (!na_mgr_) {
        na_mgr_ = new QNetworkAccessManager();
    }
}

SeafileApiClient::~SeafileApiClient()
{
    if (reply_) {
        reply_->deleteLater();
    }
}

void SeafileApiClient::get(const QUrl& url)
{
    QNetworkRequest request(url);

    if (token_.length() > 0) {
        char buf[1024];
        qsnprintf(buf, sizeof(buf), "Token %s", token_.toUtf8().data());
        request.setRawHeader(kAuthHeader, buf);
    }

    // qDebug("send request, url = %s\n, token = %s\n",
    //        request.url().toString().toUtf8().data(),
    //        request.rawHeader(kAuthHeader).data());

    reply_ = na_mgr_->get(request);

    // ignore ssl error
    reply_->ignoreSslErrors();

    connect(reply_, SIGNAL(finished()), this, SLOT(httpRequestFinished()));
}

void SeafileApiClient::post(const QUrl& url, const QByteArray& encodedParams)
{
    QNetworkRequest request(url);
    if (token_.length() > 0) {
        char buf[1024];
        qsnprintf(buf, sizeof(buf), "Token %s", token_.toUtf8().data());
        request.setRawHeader(kAuthHeader, buf);
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader, kContentTypeForm);
    reply_ = na_mgr_->post(request, encodedParams);

    // ignore ssl error
    reply_->ignoreSslErrors();

    connect(reply_, SIGNAL(finished()), this, SLOT(httpRequestFinished()));
}

void SeafileApiClient::httpRequestFinished()
{
    int code = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply_->error() != QNetworkReply::NoError) {
        qDebug("http request failed: %s\n", reply_->errorString().toUtf8().data());
        emit requestFailed(code);
        return;
    }

    if ((code / 100) != 2) {
        qDebug("request failed : status code %d\n", code);
        emit requestFailed(code);
    }

    emit requestSuccess(*reply_);
}
