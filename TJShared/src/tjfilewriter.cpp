#include "../include/tjshared.h"
using namespace tj::shared;

FileWriter::FileWriter(std::string root): _root(root) {
	TiXmlDeclaration decl("1.0","UTF-8","no");
	_document.InsertEndChild(decl);
}

FileWriter::~FileWriter() {
}

TiXmlDocument* FileWriter::GetDocument() {
	return &_document;
}

TiXmlElement* FileWriter::GetRoot() {
	return &_root;
}

void FileWriter::Save(std::string filename) {
	_document.InsertEndChild(_root);
	_document.SaveFile(filename.c_str());
}

void FileWriter::Add(Serializable* ser) {
	ser->Save(GetRoot());
}

void FileWriter::Add(ref<Serializable> ser) {
	ser->Save(GetRoot());
}

FileReader::FileReader() {
}

FileReader::~FileReader() {
}

void FileReader::Read(const std::string& filename, ref<Serializable> ser) {
	TiXmlDocument document(filename);
	if(!document.LoadFile()) Throw(TL(file_load_failed), ExceptionTypeError);

	TiXmlElement* root = document.RootElement();
	if(root==0) Throw(TL(file_format_invalid), ExceptionTypeError);
	TiXmlElement* modelElement = root->FirstChildElement("model");
	if(modelElement==0) Throw(TL(file_format_invalid), ExceptionTypeError);
	
	ser->Load(modelElement);
}