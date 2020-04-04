
vdbVec3 vdbGetForegroundColor()
{
    if (settings.global_theme == VDB_DARK_THEME)
        return vdbVec3(1,1,1);
    else
        return vdbVec3(0,0,0);
}

vdbVec3 vdbGetBackgroundColor()
{
    if (settings.global_theme == VDB_DARK_THEME)
        return vdbVec3(VDB_DARK_THEME_BACKGROUND);
    else
        return vdbVec3(VDB_BRIGHT_THEME_BACKGROUND);
}

