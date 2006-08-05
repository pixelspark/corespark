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

void FileReader::Read(std::string filename, ref<Serializable> ser) {
	TiXmlDocument document(filename);
	if(!document.LoadFile()) Throw(L"Could not load file. Please check if you have permission to read the file and that it is not in use by other applications. ", ExceptionTypeError);

	TiXmlElement* root = document.RootElement();
	if(root==0) Throw(L"The file was loaded, but there was something wrong with its structure. Check if the file isn't damaged or was created by another version of the program.", ExceptionTypeError);
	TiXmlElement* modelElement = root->FirstChildElement("model");
	if(modelElement==0) Throw(L"The file was loaded, but there was something wrong with its structure. Check if the file isn't damaged or was created by another version of the program.", ExceptionTypeError);
	
	ser->Load(modelElement);
}