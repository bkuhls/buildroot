/*!
 * @file    bandplan.h
 * @brief   header file for bandplans
 * @details definitions & function declarations
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    27.02.2014
 *
 * @note
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * @n
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * @n
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "bandplan.h"

/*!
 * @var     bandplan_NONE
 * @brief   empty bandplan
 * @details default bandplan;
 */
bandplan_t bandplan_NONE = {
    .dsl_type = "?",                /*!< unknown DSL type                   */
    .num_tone = 512,                /*!< 256 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 0,                  /*!< no bands defined                   */
};

/*!
 * @var     bandplan_ADSL_AnnexA
 * @brief   bandplan for ADSL Annex A
 * @details ITU T-REC G992-1 Annex A
 */
bandplan_t bandplan_ADSL_AnnexA = {
    .dsl_type = "ADSL",            /*!< ADSL2                              */
    .num_tone = 256,                /*!< 256 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 6,                /*!< 25.875 kHz                         */
        .up_max = 32,               /*!< 138 kHz                            */
        .down_min = 32,             /*!< 138 kHz                            */
        .down_max = 256             /*!< 1104 kHz                           */
    }
};

/*!
 * @var     bandplan_ADSL_AnnexB
 * @brief   bandplan for ADSL Annex B
 * @details ITU T-REC G992-1 Annex B
 */
bandplan_t bandplan_ADSL_AnnexB = {
    .dsl_type = "ADSL",            /*!< ADSL2                              */
    .num_tone = 256,                /*!< 256 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 28,                /*!< 25.875 kHz                         */
        .up_max = 64,               /*!< 138 kHz                            */
        .down_min = 64,             /*!< 138 kHz                            */
        .down_max = 256             /*!< 1104 kHz                           */
    }
};

/*!
 * @var     bandplan_ADSL2_AnnexA
 * @brief   bandplan for ADSL2 Annex A
 * @details ITU T-REC G992-3 Annex A
 */
bandplan_t bandplan_ADSL2_AnnexA = {
    .dsl_type = "ADSL2",            /*!< ADSL2                              */
    .num_tone = 256,                /*!< 256 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 6,                /*!< 25.875 kHz                         */
        .up_max = 32,               /*!< 138 kHz                            */
        .down_min = 32,             /*!< 138 kHz                            */
        .down_max = 256             /*!< 1104 kHz                           */
    }
};

/*!
 * @var     bandplan_ADSL2_AnnexB
 * @brief   bandplan for ADSL2 Annex B
 * @details ITU T-REC G992-3 Annex B
 */
bandplan_t bandplan_ADSL2_AnnexB = {
    .dsl_type = "ADSL2",            /*!< ADSL2                              */
    .num_tone = 256,                /*!< 256 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 28,               /*!< 120 kHz                            */
        .up_max = 64,               /*!< 276 kHz                            */
        .down_min = 64,             /*!< 276 kHz                            */
        .down_max = 256             /*!< 1104 kHz                           */
    }
};

/*!
 * @var     bandplan_ADSL2plus_AnnexA
 * @brief   bandplan for ADSL2+ Annex A
 * @details ITU T-REC G992-5 Annex A
 */
bandplan_t bandplan_ADSL2plus_AnnexA = {
    .dsl_type = "ADSL2+",           /*!< ADSL2+                             */
    .num_tone = 512,                /*!< 512 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 6,                /*!< 25.875 kHz                         */
        .up_max = 32,               /*!< 138 kHz                            */
        .down_min = 32,             /*!< 138 kHz                            */
        .down_max = 512             /*!< 2208 kHz                           */
    }
};

/*!
 * @var     bandplan_ADSL2plus_AnnexB
 * @brief   bandplan for ADSL2+ Annex B
 * @details ITU T-REC G992-5 Annex B
 */
bandplan_t bandplan_ADSL2plus_AnnexB = {
    .dsl_type = "ADSL2+",           /*!< ADSL2+                             */
    .num_tone = 512,                /*!< 512 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 28,               /*!< 120 kHz                            */
        .up_max = 64,               /*!< 276 kHz                            */
        .down_min = 64,             /*!< 276 kHz                            */
        .down_max = 512             /*!< 2208 kHz                           */
    }
};

/*!
 * @var     bandplan_ADSL2plus_AnnexJ
 * @brief   bandplan for ADSL2+ Annex J
 * @details ITU T-REC G992-5 Annex J
 */
bandplan_t bandplan_ADSL2plus_AnnexJ = {
    .dsl_type = "ADSL2+",           /*!< ADSL2+                             */
    .num_tone = 512,                /*!< 512 tones                          */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 1,                  /*!< 1 band                             */
    .band[0] = {
        .up_min = 0,                /*!< 0 kHz                              */
        .up_max = 64,               /*!< 276 kHz                            */
        .down_min = 64,             /*!< 276 kHz                            */
        .down_max = 512             /*!< 2208 kHz                           */
    }
};

/*!
 * @var     bandplan_VDSL2_AnnexB_8b_998_M2x_B
 * @brief   bandplan for VDSL2 Annex B Profil 8b
 * @details ITU T-REC G993-2 Annex B
 *          Profil 8b Bandplan 998ADE17-M2x-B
 */
bandplan_t bandplan_VDSL2_AnnexB_8b_998_M2x_B = {
    .dsl_type = "VDSL2",            /*!< VDSL2                              */
    .num_tone = 2048,               /*!< 2048 tones                         */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 2,                  /*!< 2 bands                            */
    .band[0] = {
        .up_min = 28,               /*!< 120 kHz                            */
        .up_max = 64,               /*!< 276 kHz                            */
        .down_min = 64,             /*!< 276 kHz                            */
        .down_max = 870             /*!< 3750 kHz                           */
    },
    .band[1] = {
        .up_min = 870,              /*!< 3750 kHz                           */
        .up_max = 1206,             /*!< 5200 kHz                           */
        .down_min = 1206,           /*!< 5200 kHz                           */
        .down_max = 1971,           /*!< 8500 kHz                           */
    },
};

/*!
 * @var     bandplan_VDSL2_AnnexB_17a_998ADE17_M2x_B
 * @brief   bandplan for VDSL2 Annex B Profil 8b
 * @details ITU T-REC G993-2 Annex B
 *          Profil 17a Bandplan 998ADE17-M2x-B
 */
bandplan_t bandplan_VDSL2_AnnexB_17a_998ADE17_M2x_B = {
    .dsl_type = "VDSL2",            /*!< VDSL2                              */
    .num_tone = 4096,               /*!< 4096 tones                         */
    .spacing = 4.3125,              /*!< spacing 4.3125 kHz                 */
    .num_band = 3,                  /*!< 3 bands                            */
    .band[0] = {
        .up_min = 28,               /*!< 120 kHz                            */
        .up_max = 64,               /*!< 276 kHz                            */
        .down_min = 64,             /*!< 276 kHz                            */
        .down_max = 870             /*!< 3750 kHz                           */
    },
    .band[1] = {
        .up_min = 870,              /*!< 3750 kHz                           */
        .up_max = 1206,             /*!< 5200 kHz                           */
        .down_min = 1206,           /*!< 5200 kHz                           */
        .down_max = 1971,           /*!< 8500 kHz                           */
    },
    .band[2] = {
        .up_min = 1971,             /*!< 8500 kHz                           */
        .up_max = 2783,             /*!< 12000 kHz                          */
        .down_min = 2783,           /*!< 12000 kHz                          */
        .down_max = 4096,           /*!< 17664 kHz                          */
    }
};

/* _oOo_ */
