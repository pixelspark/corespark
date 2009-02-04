#include "../include/tjcore.h"
using namespace tj::shared;


void XML::GetElementHash(const TiXmlNode* root, SecureHash& sh) {
	if(root!=0) {
		const char* text = root->Value();
		sh.AddData(text, strlen(text));

		const TiXmlNode* node = root->FirstChild();
		while(node!=0) {
			GetElementHash(node, sh);
			node = node->NextSibling();
		}
	}
}

FileWriter::FileWriter(const std::string& root): _root(GC::Hold(new TiXmlElement(root))), _document(GC::Hold(new TiXmlDocument())) {
	TiXmlDeclaration decl("1.0","UTF-8","no");
	_document->InsertEndChild(decl);
}

FileWriter::~FileWriter() {
}

strong<TiXmlDocument> FileWriter::GetDocument() {
	return _document;
}

strong<TiXmlElement> FileWriter::GetRoot() {
	return _root;
}

void FileWriter::Save(const std::string& filename) {
	ZoneEntry ze(Zones::LocalFileWriteZone);
	_document->InsertEndChild(*(ref<TiXmlElement>(_root).GetPointer()));
	_document->SaveFile(filename.c_str());
}

void FileWriter::Add(Serializable* ser) {
	ser->Save(ref<TiXmlElement>(_root).GetPointer());
}

void FileWriter::Add(ref<Serializable> ser) {
	ser->Save(ref<TiXmlElement>(_root).GetPointer());
}

FileReader::FileReader() {
}

FileReader::~FileReader() {
}

void FileReader::Read(const std::string& filename, ref<Serializable> ser) {
	ZoneEntry ze(Zones::LocalFileReadZone);

	TiXmlDocument document(filename);
	if(!document.LoadFile()) Throw(TL(file_load_failed), ExceptionTypeError);

	TiXmlElement* root = document.RootElement();
	if(root==0) Throw(TL(file_format_invalid), ExceptionTypeError);
	TiXmlElement* modelElement = root->FirstChildElement("model");
	if(modelElement==0) Throw(TL(file_format_invalid), ExceptionTypeError);
	
	ser->Load(modelElement);
}