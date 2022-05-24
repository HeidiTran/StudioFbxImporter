#pragma once

#include "fbxsdk.h"

enum MaterialProperty {
  AmbientColor,
  AmbientFactor,
  Bump,
  DiffuseColor,
  DiffuseFactor,
  DisplacementColor,
  DisplacementFactor,
  NormalMap,
  ReflectionFactor,
  Reflectivity,
  ShininessExponent,
  SpecularColor,
  SpecularFactor
};

static const char* MaterialPropertyStrings[] = {
    "AmbientColor",       "AmbientFactor",     "Bump",
    "DiffuseColor",       "DiffuseFactor",     "DisplacementColor",
    "DisplacementFactor", "NormalMap",         "ReflectionFactor",
    "reflectivity",       "ShininessExponent", "SpecularColor",
    "SpecularFactor"};

static const char* getMaterialPropertyVal(int enumIdx) {
  return MaterialPropertyStrings[enumIdx];
}

static bool isMatProp(FbxString s, int enumIdx) {
  return s == getMaterialPropertyVal(enumIdx);
}