#pragma once

typedef enum {
        Error_none = 0,
        Error_cantOpenDisplay,
        Error_cantCloseDisplay,
        Error_cantSetName,
        Error_cantMapWindow,
        Error_cantOpenFile,
        Error_cantInitFreetype,
        Error_cantLoadFont,
        Error_outOfBounds
} Error;
