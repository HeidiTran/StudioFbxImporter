#include "MTLParser.h"

MTLValue::MTLValue(QString value) { mValue = value; }

QList<float> MTLValue::toQListFloat() {
  QStringList tokens = mValue.split(" ");
  QList<float> list;
  bool ok;
  for (int i = 0; i < tokens.count(); i++) {
    list.append(tokens[i].toFloat(&ok));
    if (!ok) {
      DzHelpers::logError("Failed to convert " % tokens[i] %
                          " to a QList<float>");
    }
  }

  return list;
}

int MTLValue::toInt() {
  bool ok;
  int i = mValue.toInt(&ok);
  if (!ok) {
    DzHelpers::logError("Failed to convert " % mValue % " to an int");
  }

  return i;
}

float MTLValue::toFloat() {
  bool ok;
  float f = mValue.toFloat(&ok);
  if (!ok) {
    DzHelpers::logError("Failed to convert " % mValue % " to a float");
  }

  return f;
}

QString MTLValue::toQString() { return mValue; }

MTLParser::MTLParser() {
  mStrToMTLKey.insert("Ka", MTLKey::Ka);
  mStrToMTLKey.insert("Kd", MTLKey::Kd);
  mStrToMTLKey.insert("Ks", MTLKey::Ks);
  mStrToMTLKey.insert("Ke", MTLKey::Ke);
  mStrToMTLKey.insert("Ns", MTLKey::Ns);
  mStrToMTLKey.insert("Ni", MTLKey::Ni);
  mStrToMTLKey.insert("d", MTLKey::d);
  mStrToMTLKey.insert("illum", MTLKey::illum);
  mStrToMTLKey.insert("map_Kd", MTLKey::map_Kd);
  mStrToMTLKey.insert("map_Bump", MTLKey::map_Bump);
  mStrToMTLKey.insert("map_Ks", MTLKey::map_Ks);
  mStrToMTLKey.insert("map_Ke", MTLKey::map_Ke);
  mStrToMTLKey.insert("map_d", MTLKey::map_d);

  auto iter = mStrToMTLKey.constBegin();
  while (iter != mStrToMTLKey.constEnd()) {
    mMTLKeyToStr[iter.value()] = iter.key();
    iter++;
  }
}

QMap<QString, MTLMap> MTLParser::parse(const QString& path) {
  QMap<QString, MTLMap> nameToProperties;
  // The path should only be null if the user didn't select an MTL file path
  if (path == nullptr) {
    return nameToProperties;
  }

  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    DzHelpers::logError("Failed to open " % path % " for MTL parsing");
    return nameToProperties;
  }

  QString currMTLName = nullptr;
  QTextStream in(&f);
  while (!in.atEnd()) {
    QString line = in.readLine();
    if (line == "") {
      continue;
    }

    QStringList tokens = line.split(" ");
    if (tokens[0] != "newmtl" && currMTLName == nullptr) {
      continue;
    } else if (tokens[0] == "newmtl") {
      currMTLName = tokens[1];
      nameToProperties.insert(currMTLName, MTLMap());
    } else {
      if (!mStrToMTLKey.contains(tokens[0])) {
        DzHelpers::logError(tokens[0] +
                            " is not supported by the MTLParser currently...");
        continue;
      }

      MTLKey key = mStrToMTLKey[tokens[0]];
      MTLValue value(line.right(line.length() - tokens[0].length() - 1)
                         .replace("\\\\", "/"));  // Replace Windows separator
      nameToProperties[currMTLName].insert(key, value);
    }
  }

  logMap(nameToProperties);
  f.close();

  return nameToProperties;
}

void MTLParser::logMap(const QMap<QString, MTLMap>& map) {
  auto iter = map.begin();
  while (iter != map.end()) {
    DzHelpers::logInfo("Material: " + iter.key());
    MTLMap map1 = iter.value();
    auto iter1 = map1.begin();
    while (iter1 != map1.end()) {
      DzHelpers::logInfo(mMTLKeyToStr[iter1.key()] + ": " +
                         iter1.value().toQString());
      iter1++;
    }
    DzHelpers::logInfo("");

    iter++;
  }
}
