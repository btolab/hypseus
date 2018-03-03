/*
 * ____ DAPHNE COPYRIGHT NOTICE ____
 *
 * Copyright (C) 2001 Matt Ownby
 *
 * This file is part of DAPHNE, a laserdisc arcade game emulator
 *
 * DAPHNE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DAPHNE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include "rgb2yuv.h"

// if we aren't using the assembly version, then use the C version instead
#ifndef USE_MMX

// these are int's so they are the optimal size for the CPU to work with
unsigned int rgb2yuv_input[3] = {0}; // 8-bit values
unsigned int rgb2yuv_result_y = 0;   // 8-bit
unsigned int rgb2yuv_result_u = 0;   // 8-bit
unsigned int rgb2yuv_result_v = 0;   // 8-bit

#define R rgb2yuv_input[0]
#define G rgb2yuv_input[1]
#define B rgb2yuv_input[2]

#define Y rgb2yuv_result_y
#define U rgb2yuv_result_u
#define V rgb2yuv_result_v

static const long mul_3736[256] =
    {0,      3736,   7472,   11208,  14944,  18680,  22416,  26152,  29888,
     33624,  37360,  41096,  44832,  48568,  52304,  56040,  59776,  63512,
     67248,  70984,  74720,  78456,  82192,  85928,  89664,  93400,  97136,
     100872, 104608, 108344, 112080, 115816, 119552, 123288, 127024, 130760,
     134496, 138232, 141968, 145704, 149440, 153176, 156912, 160648, 164384,
     168120, 171856, 175592, 179328, 183064, 186800, 190536, 194272, 198008,
     201744, 205480, 209216, 212952, 216688, 220424, 224160, 227896, 231632,
     235368, 239104, 242840, 246576, 250312, 254048, 257784, 261520, 265256,
     268992, 272728, 276464, 280200, 283936, 287672, 291408, 295144, 298880,
     302616, 306352, 310088, 313824, 317560, 321296, 325032, 328768, 332504,
     336240, 339976, 343712, 347448, 351184, 354920, 358656, 362392, 366128,
     369864, 373600, 377336, 381072, 384808, 388544, 392280, 396016, 399752,
     403488, 407224, 410960, 414696, 418432, 422168, 425904, 429640, 433376,
     437112, 440848, 444584, 448320, 452056, 455792, 459528, 463264, 467000,
     470736, 474472, 478208, 481944, 485680, 489416, 493152, 496888, 500624,
     504360, 508096, 511832, 515568, 519304, 523040, 526776, 530512, 534248,
     537984, 541720, 545456, 549192, 552928, 556664, 560400, 564136, 567872,
     571608, 575344, 579080, 582816, 586552, 590288, 594024, 597760, 601496,
     605232, 608968, 612704, 616440, 620176, 623912, 627648, 631384, 635120,
     638856, 642592, 646328, 650064, 653800, 657536, 661272, 665008, 668744,
     672480, 676216, 679952, 683688, 687424, 691160, 694896, 698632, 702368,
     706104, 709840, 713576, 717312, 721048, 724784, 728520, 732256, 735992,
     739728, 743464, 747200, 750936, 754672, 758408, 762144, 765880, 769616,
     773352, 777088, 780824, 784560, 788296, 792032, 795768, 799504, 803240,
     806976, 810712, 814448, 818184, 821920, 825656, 829392, 833128, 836864,
     840600, 844336, 848072, 851808, 855544, 859280, 863016, 866752, 870488,
     874224, 877960, 881696, 885432, 889168, 892904, 896640, 900376, 904112,
     907848, 911584, 915320, 919056, 922792, 926528, 930264, 934000, 937736,
     941472, 945208, 948944, 952680};

static const long mul_9798[256] = {
    0,       9798,    19596,   29394,   39192,   48990,   58788,   68586,
    78384,   88182,   97980,   107778,  117576,  127374,  137172,  146970,
    156768,  166566,  176364,  186162,  195960,  205758,  215556,  225354,
    235152,  244950,  254748,  264546,  274344,  284142,  293940,  303738,
    313536,  323334,  333132,  342930,  352728,  362526,  372324,  382122,
    391920,  401718,  411516,  421314,  431112,  440910,  450708,  460506,
    470304,  480102,  489900,  499698,  509496,  519294,  529092,  538890,
    548688,  558486,  568284,  578082,  587880,  597678,  607476,  617274,
    627072,  636870,  646668,  656466,  666264,  676062,  685860,  695658,
    705456,  715254,  725052,  734850,  744648,  754446,  764244,  774042,
    783840,  793638,  803436,  813234,  823032,  832830,  842628,  852426,
    862224,  872022,  881820,  891618,  901416,  911214,  921012,  930810,
    940608,  950406,  960204,  970002,  979800,  989598,  999396,  1009194,
    1018992, 1028790, 1038588, 1048386, 1058184, 1067982, 1077780, 1087578,
    1097376, 1107174, 1116972, 1126770, 1136568, 1146366, 1156164, 1165962,
    1175760, 1185558, 1195356, 1205154, 1214952, 1224750, 1234548, 1244346,
    1254144, 1263942, 1273740, 1283538, 1293336, 1303134, 1312932, 1322730,
    1332528, 1342326, 1352124, 1361922, 1371720, 1381518, 1391316, 1401114,
    1410912, 1420710, 1430508, 1440306, 1450104, 1459902, 1469700, 1479498,
    1489296, 1499094, 1508892, 1518690, 1528488, 1538286, 1548084, 1557882,
    1567680, 1577478, 1587276, 1597074, 1606872, 1616670, 1626468, 1636266,
    1646064, 1655862, 1665660, 1675458, 1685256, 1695054, 1704852, 1714650,
    1724448, 1734246, 1744044, 1753842, 1763640, 1773438, 1783236, 1793034,
    1802832, 1812630, 1822428, 1832226, 1842024, 1851822, 1861620, 1871418,
    1881216, 1891014, 1900812, 1910610, 1920408, 1930206, 1940004, 1949802,
    1959600, 1969398, 1979196, 1988994, 1998792, 2008590, 2018388, 2028186,
    2037984, 2047782, 2057580, 2067378, 2077176, 2086974, 2096772, 2106570,
    2116368, 2126166, 2135964, 2145762, 2155560, 2165358, 2175156, 2184954,
    2194752, 2204550, 2214348, 2224146, 2233944, 2243742, 2253540, 2263338,
    2273136, 2282934, 2292732, 2302530, 2312328, 2322126, 2331924, 2341722,
    2351520, 2361318, 2371116, 2380914, 2390712, 2400510, 2410308, 2420106,
    2429904, 2439702, 2449500, 2459298, 2469096, 2478894, 2488692, 2498490,
};

static const long mul_19235[256] = {
    0,       19235,   38470,   57705,   76940,   96175,   115410,  134645,
    153880,  173115,  192350,  211585,  230820,  250055,  269290,  288525,
    307760,  326995,  346230,  365465,  384700,  403935,  423170,  442405,
    461640,  480875,  500110,  519345,  538580,  557815,  577050,  596285,
    615520,  634755,  653990,  673225,  692460,  711695,  730930,  750165,
    769400,  788635,  807870,  827105,  846340,  865575,  884810,  904045,
    923280,  942515,  961750,  980985,  1000220, 1019455, 1038690, 1057925,
    1077160, 1096395, 1115630, 1134865, 1154100, 1173335, 1192570, 1211805,
    1231040, 1250275, 1269510, 1288745, 1307980, 1327215, 1346450, 1365685,
    1384920, 1404155, 1423390, 1442625, 1461860, 1481095, 1500330, 1519565,
    1538800, 1558035, 1577270, 1596505, 1615740, 1634975, 1654210, 1673445,
    1692680, 1711915, 1731150, 1750385, 1769620, 1788855, 1808090, 1827325,
    1846560, 1865795, 1885030, 1904265, 1923500, 1942735, 1961970, 1981205,
    2000440, 2019675, 2038910, 2058145, 2077380, 2096615, 2115850, 2135085,
    2154320, 2173555, 2192790, 2212025, 2231260, 2250495, 2269730, 2288965,
    2308200, 2327435, 2346670, 2365905, 2385140, 2404375, 2423610, 2442845,
    2462080, 2481315, 2500550, 2519785, 2539020, 2558255, 2577490, 2596725,
    2615960, 2635195, 2654430, 2673665, 2692900, 2712135, 2731370, 2750605,
    2769840, 2789075, 2808310, 2827545, 2846780, 2866015, 2885250, 2904485,
    2923720, 2942955, 2962190, 2981425, 3000660, 3019895, 3039130, 3058365,
    3077600, 3096835, 3116070, 3135305, 3154540, 3173775, 3193010, 3212245,
    3231480, 3250715, 3269950, 3289185, 3308420, 3327655, 3346890, 3366125,
    3385360, 3404595, 3423830, 3443065, 3462300, 3481535, 3500770, 3520005,
    3539240, 3558475, 3577710, 3596945, 3616180, 3635415, 3654650, 3673885,
    3693120, 3712355, 3731590, 3750825, 3770060, 3789295, 3808530, 3827765,
    3847000, 3866235, 3885470, 3904705, 3923940, 3943175, 3962410, 3981645,
    4000880, 4020115, 4039350, 4058585, 4077820, 4097055, 4116290, 4135525,
    4154760, 4173995, 4193230, 4212465, 4231700, 4250935, 4270170, 4289405,
    4308640, 4327875, 4347110, 4366345, 4385580, 4404815, 4424050, 4443285,
    4462520, 4481755, 4500990, 4520225, 4539460, 4558695, 4577930, 4597165,
    4616400, 4635635, 4654870, 4674105, 4693340, 4712575, 4731810, 4751045,
    4770280, 4789515, 4808750, 4827985, 4847220, 4866455, 4885690, 4904925,
};

static const long mul_18514[512] =
    {-4739584, -4721070, -4702556, -4684042, -4665528, -4647014, -4628500,
     -4609986, -4591472, -4572958, -4554444, -4535930, -4517416, -4498902,
     -4480388, -4461874, -4443360, -4424846, -4406332, -4387818, -4369304,
     -4350790, -4332276, -4313762, -4295248, -4276734, -4258220, -4239706,
     -4221192, -4202678, -4184164, -4165650, -4147136, -4128622, -4110108,
     -4091594, -4073080, -4054566, -4036052, -4017538, -3999024, -3980510,
     -3961996, -3943482, -3924968, -3906454, -3887940, -3869426, -3850912,
     -3832398, -3813884, -3795370, -3776856, -3758342, -3739828, -3721314,
     -3702800, -3684286, -3665772, -3647258, -3628744, -3610230, -3591716,
     -3573202, -3554688, -3536174, -3517660, -3499146, -3480632, -3462118,
     -3443604, -3425090, -3406576, -3388062, -3369548, -3351034, -3332520,
     -3314006, -3295492, -3276978, -3258464, -3239950, -3221436, -3202922,
     -3184408, -3165894, -3147380, -3128866, -3110352, -3091838, -3073324,
     -3054810, -3036296, -3017782, -2999268, -2980754, -2962240, -2943726,
     -2925212, -2906698, -2888184, -2869670, -2851156, -2832642, -2814128,
     -2795614, -2777100, -2758586, -2740072, -2721558, -2703044, -2684530,
     -2666016, -2647502, -2628988, -2610474, -2591960, -2573446, -2554932,
     -2536418, -2517904, -2499390, -2480876, -2462362, -2443848, -2425334,
     -2406820, -2388306, -2369792, -2351278, -2332764, -2314250, -2295736,
     -2277222, -2258708, -2240194, -2221680, -2203166, -2184652, -2166138,
     -2147624, -2129110, -2110596, -2092082, -2073568, -2055054, -2036540,
     -2018026, -1999512, -1980998, -1962484, -1943970, -1925456, -1906942,
     -1888428, -1869914, -1851400, -1832886, -1814372, -1795858, -1777344,
     -1758830, -1740316, -1721802, -1703288, -1684774, -1666260, -1647746,
     -1629232, -1610718, -1592204, -1573690, -1555176, -1536662, -1518148,
     -1499634, -1481120, -1462606, -1444092, -1425578, -1407064, -1388550,
     -1370036, -1351522, -1333008, -1314494, -1295980, -1277466, -1258952,
     -1240438, -1221924, -1203410, -1184896, -1166382, -1147868, -1129354,
     -1110840, -1092326, -1073812, -1055298, -1036784, -1018270, -999756,
     -981242,  -962728,  -944214,  -925700,  -907186,  -888672,  -870158,
     -851644,  -833130,  -814616,  -796102,  -777588,  -759074,  -740560,
     -722046,  -703532,  -685018,  -666504,  -647990,  -629476,  -610962,
     -592448,  -573934,  -555420,  -536906,  -518392,  -499878,  -481364,
     -462850,  -444336,  -425822,  -407308,  -388794,  -370280,  -351766,
     -333252,  -314738,  -296224,  -277710,  -259196,  -240682,  -222168,
     -203654,  -185140,  -166626,  -148112,  -129598,  -111084,  -92570,
     -74056,   -55542,   -37028,   -18514,   0,        18514,    37028,
     55542,    74056,    92570,    111084,   129598,   148112,   166626,
     185140,   203654,   222168,   240682,   259196,   277710,   296224,
     314738,   333252,   351766,   370280,   388794,   407308,   425822,
     444336,   462850,   481364,   499878,   518392,   536906,   555420,
     573934,   592448,   610962,   629476,   647990,   666504,   685018,
     703532,   722046,   740560,   759074,   777588,   796102,   814616,
     833130,   851644,   870158,   888672,   907186,   925700,   944214,
     962728,   981242,   999756,   1018270,  1036784,  1055298,  1073812,
     1092326,  1110840,  1129354,  1147868,  1166382,  1184896,  1203410,
     1221924,  1240438,  1258952,  1277466,  1295980,  1314494,  1333008,
     1351522,  1370036,  1388550,  1407064,  1425578,  1444092,  1462606,
     1481120,  1499634,  1518148,  1536662,  1555176,  1573690,  1592204,
     1610718,  1629232,  1647746,  1666260,  1684774,  1703288,  1721802,
     1740316,  1758830,  1777344,  1795858,  1814372,  1832886,  1851400,
     1869914,  1888428,  1906942,  1925456,  1943970,  1962484,  1980998,
     1999512,  2018026,  2036540,  2055054,  2073568,  2092082,  2110596,
     2129110,  2147624,  2166138,  2184652,  2203166,  2221680,  2240194,
     2258708,  2277222,  2295736,  2314250,  2332764,  2351278,  2369792,
     2388306,  2406820,  2425334,  2443848,  2462362,  2480876,  2499390,
     2517904,  2536418,  2554932,  2573446,  2591960,  2610474,  2628988,
     2647502,  2666016,  2684530,  2703044,  2721558,  2740072,  2758586,
     2777100,  2795614,  2814128,  2832642,  2851156,  2869670,  2888184,
     2906698,  2925212,  2943726,  2962240,  2980754,  2999268,  3017782,
     3036296,  3054810,  3073324,  3091838,  3110352,  3128866,  3147380,
     3165894,  3184408,  3202922,  3221436,  3239950,  3258464,  3276978,
     3295492,  3314006,  3332520,  3351034,  3369548,  3388062,  3406576,
     3425090,  3443604,  3462118,  3480632,  3499146,  3517660,  3536174,
     3554688,  3573202,  3591716,  3610230,  3628744,  3647258,  3665772,
     3684286,  3702800,  3721314,  3739828,  3758342,  3776856,  3795370,
     3813884,  3832398,  3850912,  3869426,  3887940,  3906454,  3924968,
     3943482,  3961996,  3980510,  3999024,  4017538,  4036052,  4054566,
     4073080,  4091594,  4110108,  4128622,  4147136,  4165650,  4184164,
     4202678,  4221192,  4239706,  4258220,  4276734,  4295248,  4313762,
     4332276,  4350790,  4369304,  4387818,  4406332,  4424846,  4443360,
     4461874,  4480388,  4498902,  4517416,  4535930,  4554444,  4572958,
     4591472,  4609986,  4628500,  4647014,  4665528,  4684042,  4702556,
     4721070};

static const long mul_23364[512] =
    {-5981184, -5957820, -5934456, -5911092, -5887728, -5864364, -5841000,
     -5817636, -5794272, -5770908, -5747544, -5724180, -5700816, -5677452,
     -5654088, -5630724, -5607360, -5583996, -5560632, -5537268, -5513904,
     -5490540, -5467176, -5443812, -5420448, -5397084, -5373720, -5350356,
     -5326992, -5303628, -5280264, -5256900, -5233536, -5210172, -5186808,
     -5163444, -5140080, -5116716, -5093352, -5069988, -5046624, -5023260,
     -4999896, -4976532, -4953168, -4929804, -4906440, -4883076, -4859712,
     -4836348, -4812984, -4789620, -4766256, -4742892, -4719528, -4696164,
     -4672800, -4649436, -4626072, -4602708, -4579344, -4555980, -4532616,
     -4509252, -4485888, -4462524, -4439160, -4415796, -4392432, -4369068,
     -4345704, -4322340, -4298976, -4275612, -4252248, -4228884, -4205520,
     -4182156, -4158792, -4135428, -4112064, -4088700, -4065336, -4041972,
     -4018608, -3995244, -3971880, -3948516, -3925152, -3901788, -3878424,
     -3855060, -3831696, -3808332, -3784968, -3761604, -3738240, -3714876,
     -3691512, -3668148, -3644784, -3621420, -3598056, -3574692, -3551328,
     -3527964, -3504600, -3481236, -3457872, -3434508, -3411144, -3387780,
     -3364416, -3341052, -3317688, -3294324, -3270960, -3247596, -3224232,
     -3200868, -3177504, -3154140, -3130776, -3107412, -3084048, -3060684,
     -3037320, -3013956, -2990592, -2967228, -2943864, -2920500, -2897136,
     -2873772, -2850408, -2827044, -2803680, -2780316, -2756952, -2733588,
     -2710224, -2686860, -2663496, -2640132, -2616768, -2593404, -2570040,
     -2546676, -2523312, -2499948, -2476584, -2453220, -2429856, -2406492,
     -2383128, -2359764, -2336400, -2313036, -2289672, -2266308, -2242944,
     -2219580, -2196216, -2172852, -2149488, -2126124, -2102760, -2079396,
     -2056032, -2032668, -2009304, -1985940, -1962576, -1939212, -1915848,
     -1892484, -1869120, -1845756, -1822392, -1799028, -1775664, -1752300,
     -1728936, -1705572, -1682208, -1658844, -1635480, -1612116, -1588752,
     -1565388, -1542024, -1518660, -1495296, -1471932, -1448568, -1425204,
     -1401840, -1378476, -1355112, -1331748, -1308384, -1285020, -1261656,
     -1238292, -1214928, -1191564, -1168200, -1144836, -1121472, -1098108,
     -1074744, -1051380, -1028016, -1004652, -981288,  -957924,  -934560,
     -911196,  -887832,  -864468,  -841104,  -817740,  -794376,  -771012,
     -747648,  -724284,  -700920,  -677556,  -654192,  -630828,  -607464,
     -584100,  -560736,  -537372,  -514008,  -490644,  -467280,  -443916,
     -420552,  -397188,  -373824,  -350460,  -327096,  -303732,  -280368,
     -257004,  -233640,  -210276,  -186912,  -163548,  -140184,  -116820,
     -93456,   -70092,   -46728,   -23364,   0,        23364,    46728,
     70092,    93456,    116820,   140184,   163548,   186912,   210276,
     233640,   257004,   280368,   303732,   327096,   350460,   373824,
     397188,   420552,   443916,   467280,   490644,   514008,   537372,
     560736,   584100,   607464,   630828,   654192,   677556,   700920,
     724284,   747648,   771012,   794376,   817740,   841104,   864468,
     887832,   911196,   934560,   957924,   981288,   1004652,  1028016,
     1051380,  1074744,  1098108,  1121472,  1144836,  1168200,  1191564,
     1214928,  1238292,  1261656,  1285020,  1308384,  1331748,  1355112,
     1378476,  1401840,  1425204,  1448568,  1471932,  1495296,  1518660,
     1542024,  1565388,  1588752,  1612116,  1635480,  1658844,  1682208,
     1705572,  1728936,  1752300,  1775664,  1799028,  1822392,  1845756,
     1869120,  1892484,  1915848,  1939212,  1962576,  1985940,  2009304,
     2032668,  2056032,  2079396,  2102760,  2126124,  2149488,  2172852,
     2196216,  2219580,  2242944,  2266308,  2289672,  2313036,  2336400,
     2359764,  2383128,  2406492,  2429856,  2453220,  2476584,  2499948,
     2523312,  2546676,  2570040,  2593404,  2616768,  2640132,  2663496,
     2686860,  2710224,  2733588,  2756952,  2780316,  2803680,  2827044,
     2850408,  2873772,  2897136,  2920500,  2943864,  2967228,  2990592,
     3013956,  3037320,  3060684,  3084048,  3107412,  3130776,  3154140,
     3177504,  3200868,  3224232,  3247596,  3270960,  3294324,  3317688,
     3341052,  3364416,  3387780,  3411144,  3434508,  3457872,  3481236,
     3504600,  3527964,  3551328,  3574692,  3598056,  3621420,  3644784,
     3668148,  3691512,  3714876,  3738240,  3761604,  3784968,  3808332,
     3831696,  3855060,  3878424,  3901788,  3925152,  3948516,  3971880,
     3995244,  4018608,  4041972,  4065336,  4088700,  4112064,  4135428,
     4158792,  4182156,  4205520,  4228884,  4252248,  4275612,  4298976,
     4322340,  4345704,  4369068,  4392432,  4415796,  4439160,  4462524,
     4485888,  4509252,  4532616,  4555980,  4579344,  4602708,  4626072,
     4649436,  4672800,  4696164,  4719528,  4742892,  4766256,  4789620,
     4812984,  4836348,  4859712,  4883076,  4906440,  4929804,  4953168,
     4976532,  4999896,  5023260,  5046624,  5069988,  5093352,  5116716,
     5140080,  5163444,  5186808,  5210172,  5233536,  5256900,  5280264,
     5303628,  5326992,  5350356,  5373720,  5397084,  5420448,  5443812,
     5467176,  5490540,  5513904,  5537268,  5560632,  5583996,  5607360,
     5630724,  5654088,  5677452,  5700816,  5724180,  5747544,  5770908,
     5794272,  5817636,  5841000,  5864364,  5887728,  5911092,  5934456,
     5957820};

// Function contributed by 'Buddabing'
void rgb2yuv()
{
    Y = ((mul_9798[R] + mul_19235[G] + mul_3736[B]) >> 15);
    U = ((mul_18514[B - Y + 256] >> 15) + 128);
    V = ((mul_23364[R - Y + 256] >> 15) + 128);
}

/*
// a faster, less accurate version of the RGB->YUV conversion formula
// (faster because no floating point is used)
void rgb2yuv()
{
    rgb2yuv_result_y = (unsigned char) (((9798*R) + (19235*G) + (3736*B)) >>
15);
    rgb2yuv_result_u = (unsigned char) ((((B-rgb2yuv_result_y)*18514) >> 15) +
128);
    rgb2yuv_result_v = (unsigned char) ((((R-rgb2yuv_result_y)*23364) >> 15) +
128);
}
*/

#endif // not MMX_RGB2YUV
