#pragma once

#include "DzHelpers.h"

enum class MTLKey {
	Ka,
	Kd,
	Ks,
	Ke,
	Ns,
	Ni,
	d,
	illum,
	map_Kd,
	map_Bump,
	map_Ks,
	map_Ke,
	map_d
};

class MTLValue {
	private:
		QString mValue;
	public:
		MTLValue(QString value);

		QList<float> toQListFloat();
		int toInt();
		float toFloat();
		QString toQString();
};

using MTLMap = QMap<MTLKey, MTLValue>;

class MTLParser {
	private:
		QMap<QString, MTLKey> mStrToMTLKey;
		QMap<MTLKey, QString> mMTLKeyToStr;

		void logMap(const QMap<QString, MTLMap>& map);
	public:
		MTLParser();
		QMap<QString, MTLMap> parse(const QString &path);
};

