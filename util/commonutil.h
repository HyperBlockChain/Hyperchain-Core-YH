/*copyright 2016-2018 hyperchain.net  (Hyperchain)
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

#ifndef COMMONUTIL_H
#define COMMONUTIL_H

#include "HChainP2PManager/headers/inter_public.h"
#include <string>
#include <QString>
#include <QVariantMap>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

std::string utf8_to_ansi(const std::string& utf8);
std::string ansi_to_utf8(const std::string& ansi);

#endif

QString imageRoot();
QString logoPath();
QString ChainStatusPath();
QString ChainRunStatusPath();
QString NodeConnectStatusPath();
QString PoePath();

QString TokenPath();
QString PassPath();
QString ScriptPath();
QString ContractPath();
QString AboutLogoPath();

QString translationRoot();
QString Trans_En_Path();
QString Trans_Zh_Path();

QString closeBtnStyle();
QString minBtnStyle();

QString secsToHourMinSecs(quint64 secs);

void convertEvidenceVariantMap2Struct(TEVIDENCEINFO* dest, const QVariantMap& evidenceInfo);
void convertEvidenceStruct2VariantMap(QVariantMap& info, const TEVIDENCEINFO* dest);

#endif
