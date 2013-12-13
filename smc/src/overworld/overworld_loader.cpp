#include "overworld_loader.h"
#include "overworld_description_loader.h"
#include "overworld.h"

namespace fs = boost::filesystem;
using namespace SMC;

cOverworldLoader::cOverworldLoader()
	: xmlpp::SaxParser()
{
	mp_world = NULL;
}

cOverworldLoader::~cOverworldLoader()
{
	// Do not delete the cLevel instance — it is used by the
	// caller and delted by him.
	mp_world = NULL;
}

cOverworld* cOverworldLoader::Get_Overworld()
{
	return mp_world;
}

/***************************************
 * SAX parser callbacks
 ***************************************/

void cOverworldLoader::parse_dir(fs::path dirname)
{
	m_worlddir = dirname;
	xmlpp::SaxParser::parse_file(path_to_utf8(dirname / utf8_to_path("world.xml")));
}

void cOverworldLoader::on_start_document()
{
	if (mp_world)
		throw("Restarted XML parser after already starting it."); // FIXME: proper exception

	// Load the description XML file
	cOverworldDescriptionLoader descloader;
	cOverworld_description* p_desc = NULL;
	descloader.parse_file(m_worlddir / utf8_to_path("description.xml"));
	p_desc = descloader.Get_Overworld_Description();
	p_desc->Set_Path(m_worlddir);

	mp_world = new cOverworld();
	mp_world->Replace_Description(descloader.Get_Overworld_Description()); // FIXME: OO-violating post-initialization
}

void cOverworldLoader::on_end_document()
{
	// engine version entry not set
	if (mp_world->m_engine_version < 0)
		mp_world->m_engine_version = 0;
}

void cOverworldLoader::on_start_element(const Glib::ustring& name, const xmlpp::SaxParser::AttributeList& properties)
{
	if (name == "property" || name == "Property") {
		std::string key;
		std::string value;

		/* Collect all the <property> elements for the surrounding
		 * mayor element (like <settings> or <sprite>). When the
		 * surrounding element is closed, the results are handled
		 * in on_end_element(). */
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = properties.begin(); iter != properties.end(); iter++) {
			xmlpp::SaxParser::Attribute attr = *iter;

			if (attr.name == "name")
				key = attr.value;
			else if (attr.name == "value")
				value = attr.value;
		}

		m_current_properties[key] = value;
	}
}

void cOverworldLoader::on_end_element(const Glib::ustring& name)
{
	// <property> tags are parsed cumulatively in on_start_element()
	// so all have been collected when the surrounding element
	// terminates here.
	if (name == "property" || name == "Property")
		return;

	// Everything handled, so we can now safely clear the
	// collected <property> element values for the next
	// tag.
	m_current_properties.clear();
}
