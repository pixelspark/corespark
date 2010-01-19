#include "../include/tjauthorizer.h"
using namespace tj::shared;
using namespace tj::np;

/** Authorizer **/
Authorizer::Authorizer() {
	_privateKey = (unsigned int)Util::RandomInt();
}

Authorizer::~Authorizer() {
}

Authorizer::Authorizer(unsigned int pk): _privateKey(pk) {
}

SecurityToken Authorizer::CreateToken(const String& object) {
	Hash h;
	return (SecurityToken)h.Calculate(StringifyHex(_privateKey)+object);
}

bool Authorizer::CheckToken(const String& object, SecurityToken token) {
	SecurityToken generated = CreateToken(object);
	return generated == token;
}