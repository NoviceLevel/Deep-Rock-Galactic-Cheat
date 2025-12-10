#pragma once
#include "../LocaleStructs.h"

std::vector<LocaleData> ChineseLocales = {
	// 主菜单
	{HASH("CHEAT"), "作弊菜单"},
	{HASH("UNLOAD_BTN"), "卸载"},
	{HASH("CONSOLE_SHOW"), "显示控制台"},
	{HASH("CONSOLE_HIDE"), "隐藏控制台"},
	{HASH("WATER_MARK"), "水印"},
	{HASH("WATER_MARK_FPS"), "水印FPS"},
	{HASH("SAVE_CONFIG"), "保存配置"},
	{HASH("LOAD_CONFIG"), "加载配置"},
	{HASH("LANGUAGE"), "语言"},

	// 自瞄
	{HASH("AIMBOT"), "自瞄"},
	{HASH("AIMBOT_AUTO_FIRE"), "自动射击"},
	{HASH("AIMBOT_KEY"), "自瞄按键"},
	{HASH("MAGIC_BULLET"), "魔法子弹"},
	{HASH("MULTI_TARGET"), "多目标"},
	{HASH("AIMBOT_FOV"), "视野范围"},
	{HASH("AIMBOT_KEY_MODE"), "切换模式"},

	// ESP透视
	{HASH("ESP"), "透视"},
	{HASH("ESP_ENABLE"), "启用透视"},
	{HASH("PLAYER_LIST"), "玩家列表"},
	{HASH("ESP_ENEMIES"), "敌人"},
	{HASH("ESP_FRIENDLIES"), "友军"},
	{HASH("ESP_PLAYERS"), "玩家"},
	{HASH("ESP_OBJECTIVES"), "目标"},
	{HASH("ESP_SPECIAL_STRUCTURES"), "特殊建筑"},
	{HASH("ESP_RESOURCE_CHUNKS"), "资源矿块"},
	{HASH("ESP_ACCURATE_BOX"), "精确方框"},
	{HASH("ESP_BOX"), "方框"},
	{HASH("ESP_NAME"), "名称"},
	{HASH("ESP_DISTANCE"), "距离"},
	{HASH("ESP_HEALTH_BAR"), "血条"},
	{HASH("ESP_ARMOR_BAR"), "护甲条"},
	{HASH("ESP_FLAG_INVINCIBLE"), "无敌标记"},
	{HASH("ESP_DEBUG"), "调试透视"},
	{HASH("ESP_DEBUG_COLOR"), "调试颜色"},
	{HASH("ESP_MAX_DISTANCE"), "最大距离"},
	{HASH("ESP_INVINCIBLE_FLAG_TEXT"), "无敌"},

	// 玩家修改
	{HASH("GODMODE"), "上帝模式"},
	{HASH("RUNNING_SPEED"), "奔跑速度"},
	{HASH("FLY_HACK"), "喷气背包"},
	{HASH("FLY_FORCE"), "推力"},
	{HASH("NAME_CHANGER"), "名称修改"},
	{HASH("NAME_CHANGER_SET"), "设置名称"},

	// 武器修改
	{HASH("INFINITE_AMMO"), "无限弹药"},
	{HASH("NO_OVERHEATING"), "无过热"},
	{HASH("NO_RELOAD"), "无需换弹"},
	{HASH("NO_RECOIL"), "无后坐力"},
	{HASH("GRAPPLE_RESTRICTIONS"), "无抓钩限制"},
	{HASH("GRAPPLE_MAX_SPEED"), "速度"},

	// 杂项
	{HASH("FULLBRIGHT"), "全亮"}
};

LocalizationData Chinese{ "简体中文", HASH("CHN"), &ChineseFont, &ChineseFontESP, ChineseLocales };
