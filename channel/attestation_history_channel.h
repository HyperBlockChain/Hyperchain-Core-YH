/*copyright 2016-2018 hyperchain.net (Hyperchain)

Distributed under the MIT software license, see the accompanying
file COPYING or https://opensource.org/licenses/MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef ATTESTATION_HISTORY_CHANNEL_H
#define ATTESTATION_HISTORY_CHANNEL_H

#include "HChainP2PManager/headers/inter_public.h"
#include "HChainP2PManager/interface/QtInterface.h"
#include <QObject>
#include <QVariantMap>
#include <QSharedPointer>

class attestation_history_channel : public QObject
{
    Q_OBJECT

public:
    explicit attestation_history_channel(QObject *parent = 0);

public slots:
    void onHtmlReady();

    //获取全部存证记录条数
    quint64 getEvidenceCount();

    //分页获取获取存证记录
    void getEvidence(int page = 0, int pageSize = 20);
    void addEvidence(QSharedPointer<TEVIDENCEINFO> evidence);
    void updateEvidence(std::string hash, TEVIDENCEINFO* evi, int type = 1);

    //显示单个存证记录
    void checkEvidenceInfo(QVariantMap evidenceInfo);

    //删除存证记录.传入参数为quint64的数组
    void delMultiEvidence(QVariant blockNumArray);

    //导出全部存证记录
    void exportAllEvidence();

    //导出给定存证记录
    void exportEvidence(QVariant blockNumArray);

    //验证存证记录
    void verifyEvidence(QVariantMap evidenceInfo);


signals:
    void sigGetEvidence(QVariant arr);
    void sigChangeLang(int lang);
    void sigAddEvidence(QVariantMap evidenceInfo);
    void sigUpdateEvidence(QString hash, QVariantMap evidenceInfo, int type);
};

#endif // ATTESTATION_HISTORY_CHANNEL_H