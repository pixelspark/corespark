#include "../include/tjserializable.h"
#include "../include/tjzone.h"
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

/** TaggedObject **/
TaggedObject::TaggedObject(ref<Serializable> sr): _original(sr) {
}

void TaggedObject::Save(TiXmlElement* save) {
	std::set<String>::const_iterator it = _tags.begin();
	while(it!=_tags.end()) {
		const String& tag = *it;
		save->SetAttribute(Mbs(tag), 1);
		++it;
	}
	
	TiXmlElement wrapper("tagged-object");
	if(_original) {
		_original->Save(&wrapper);
	}
	save->InsertEndChild(wrapper);
}

void TaggedObject::Load(TiXmlElement* load) {	
	TiXmlElement* wrapper = load->FirstChildElement("tagged-object");
	if(wrapper!=0) {
		TiXmlAttribute* att = load->FirstAttribute();
		while(att!=0) {
			String name = Wcs(att->Name());
			_tags.insert(name);
			att = att->Next();
		}
		
		if(_original) _original->Load(wrapper);
	}
	else {
		if(_original) _original->Load(load);
	}
}

void TaggedObject::SetTag(const String& tag, bool f) {
	if(f) {
		_tags.insert(tag);
	}
	else {
		std::set<String>::iterator it = _tags.find(tag);
		if(it!=_tags.end()) {
			_tags.erase(it);
		}
	}
}

bool TaggedObject::HasTag(const String& tag) {
	return _tags.find(tag) != _tags.end();
}

/** GenericObject **/
GenericObject::GenericObject(): _element("object") {
}

GenericObject::~GenericObject() {
}

void GenericObject::Save(TiXmlElement* you) {
	you->InsertEndChild(_element);
}

void GenericObject::Load(TiXmlElement* you) {
	_element = *you;
}
