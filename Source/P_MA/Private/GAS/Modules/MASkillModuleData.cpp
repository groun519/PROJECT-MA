// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/MASkillModuleData.h"

#include "Inventory/MAItemTypes.h"

FSkillData::FSkillData():
	GrantedAbility{nullptr},
	SkillMontage{nullptr},
	VFXDataSet{nullptr}
	//SkillIcon{nullptr}
{
	ItemType = EMAItemType::Skill;
}
