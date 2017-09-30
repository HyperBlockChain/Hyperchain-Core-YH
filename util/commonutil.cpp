/*Copyright 2017 hyperchain.net  (Hyper Block Chain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT.
/*
/*Permission is hereby granted, free of charge, to any person obtaining a copy of this 
/*software and associated documentation files (the "Software"), to deal in the Software
/*without restriction, including without limitation the rights to use, copy, modify, merge,
/*publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
/*to whom the Software is furnished to do so, subject to the following conditions:
/*
/*The above copyright notice and this permission notice shall be included in all copies or
/*substantial portions of the Software.
/*
/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
/*INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/*PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/*FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/*OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/*DEALINGS IN THE SOFTWARE.
*/

#include "commonutil.h"
#include <QApplication>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
std::string utf8_to_ansi(const std::string& utf8)
{
    int wn = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), NULL, 0);
    LPWSTR pwB = new WCHAR[wn];
    wn = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), pwB, wn);

    int an = WideCharToMultiByte(CP_ACP, 0, pwB, wn, NULL, 0, NULL, NULL);
    LPSTR pB = new CHAR[an];
    an = WideCharToMultiByte(CP_ACP, 0, pwB, wn, pB, an, NULL, NULL);

    std::string tmp;
    tmp.assign(pB, an);

    delete[] pwB;
    delete[] pB;

    return tmp;
}

std::string ansi_to_utf8(const std::string& ansi)
{
    int wn = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), NULL, 0);
    LPWSTR pwB = new WCHAR[wn];
    wn = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), pwB, wn);

    int an = WideCharToMultiByte(CP_UTF8, 0, pwB, wn, NULL, 0, NULL, NULL);
    LPSTR pB = new CHAR[an];
    an = WideCharToMultiByte(CP_UTF8, 0, pwB, wn, pB, an, NULL, NULL);

    std::string tmp;
    tmp.assign(pB, an);

    delete[] pwB;
    delete[] pB;

    return tmp;
}

#endif

QString imageRoot()
{
#ifdef Q_OS_LINUX
	return QString(":/image/image");
#else

#ifdef QT_DEBUG
#if defined(Q_OS_WIN)
	QString uiroot = QString("%1/%2").arg(QApplication::applicationDirPath()).arg("../../image");
#else
	QString uiroot = QString("%1/%2").arg(QApplication::applicationDirPath()).arg("../image");
#endif
#else
	QString uiroot = QString("%1/%2").arg(QApplication::applicationDirPath()).arg("image");
#endif

	return uiroot;
#endif
}

QString logoPath()
{
     QString logoPath = QString("%1/icon.jpg").arg(imageRoot());
     return logoPath;
}

QString closeBtnStyle()
{
    QString uiroot = imageRoot();

    QString closeNormal = QString("%1/close_a.png").arg(uiroot);
    QString closeHover = QString("%1/close_hover.png").arg(uiroot);
    QString closePress = QString("%1/close_press.png").arg(uiroot);

    QString closeStyle = QString("QPushButton{border-image: url(%1);}"
                            "QPushButton:hover{border-image: url(%2);}"
                            "QPushButton:pressed{border-image: url(%3);}").arg(closeNormal).arg(closeHover).arg(closePress);

    return closeStyle;
}

QString minBtnStyle()
{
    QString uiroot = imageRoot();

    QString minNormal = QString("%1/zoomin_a.png").arg(uiroot);
    QString minHover = QString("%1/zoomin_hover.png").arg(uiroot);
    QString minPress = QString("%1/zoomin_press.png").arg(uiroot);

    QString minStyle = QString("QPushButton{border-image: url(%1);}"
                            "QPushButton:hover{border-image: url(%2);}"
                            "QPushButton:pressed{border-image: url(%3);}").arg(minNormal).arg(minHover).arg(minPress);

    return minStyle;
}

QString secsToHourMinSecs(quint64 secs)
{
    int hours = secs / 3600;

    int rest = secs % 3600;
    int mins = rest / 60;

    secs = rest % 60;

    if(hours > 0){
        return QString("%1:%2:%3").arg(hours).arg(mins).arg(secs);
    }else if(mins > 0){
        return QString("%1:%2").arg(mins).arg(secs);
    }

    return QString("%1").arg(secs);
}

void convertEvidenceVariantMap2Struct(TEVIDENCEINFO *dest, const QVariantMap &evidenceInfo)
{
    if(nullptr == dest){
        return;
    }

    dest->cFileName  = evidenceInfo["cFileName"].toString().toStdString();
    dest->cCustomInfo= evidenceInfo["cCustomInfo"].toString().toStdString();
    dest->cRightOwner= evidenceInfo["cRightOwner"].toString().toStdString();
    dest->cFileHash  = evidenceInfo["cFileHash"].toString().toStdString();
    dest->iFileState = evidenceInfo["iFileState"].toInt();
    dest->tRegisTime = evidenceInfo["tRegisTime"].toULongLong();
    dest->iFileSize  = evidenceInfo["iFileSize"].toULongLong();
}

void convertEvidenceStruct2VariantMap(QVariantMap &info, const TEVIDENCEINFO *dest)
{
    if(nullptr == dest){
        return;
    }

    info["cFileName"] = QString::fromStdString(dest->cFileName);
    info["cCustomInfo"] = QString::fromStdString(dest->cCustomInfo);
    info["cRightOwner"] = QString::fromStdString(dest->cRightOwner);
    info["cFileHash"] = QString::fromStdString(dest->cFileHash);
    info["iFileState"] = dest->iFileState;
    info["tRegisTime"] = dest->tRegisTime;
    info["iFileSize"] = dest->iFileSize;
}
