#pragma once
#include "cbase.h"

class CDispPoint : public CPointEntity
{
public:
	DECLARE_CLASS(CDispPoint, CPointEntity);
	DECLARE_DATADESC();

	void Spawn(void);

	string_t string; //if you rename this, rename it in define_keyfield and in weapon_displacer around line 122 too!!!
};
LINK_ENTITY_TO_CLASS(point_displace, CDispPoint); //if you change the name(point_displace) to something else, don't forget to change it in displacer's code too!

BEGIN_DATADESC(CDispPoint)
DEFINE_KEYFIELD(string, FIELD_STRING, "target"),
END_DATADESC();
