////  tl_to_ubv: conversion from effective temperature and luminosity to
////             Johnston UBVRI.
////             For single stars and for binaries.
////
//// Options:     -T    Temperature [Kelvin] of primary star [no default]
////              -t    Temperature [Kelvin] of secondary star [no default]
////              -L    Luminosity [Lsun] of primary star [no default]
////              -l    Luminosity [Lsun] of secondary star [no default]
////              -M    Mass [Msun] of primary star [no default]
////              -m    Mass [Msun] of secondary star [no default]
////

// version 1:  Okt 1998     Simon Portegies Zwart
//                          e-mail: spz@grape.c.u-tokyo.ac.jp
//                          Department of General System Studies
//                          College of Arts and Science, 
//                          University of Tokyo, 3-8-1 Komaba,	  
//                          Meguro-ku, Tokyo 153, Japan

#include "dyn.h"
#include "stdfunc.h"
#include "sstar_to_dyn.h"
#include "star_support.h"

#ifndef TOOLBOX

void ltm_to_ubvri(const real logl,
		  const real logt,
		  const real mass,
		  real& U,
		  real& B,
		  real& V,
		  real& R,
		  real& I) {

  real bc[4], ub[4], ri[4], bv[4], vr[4];
  real mv, uminb, bminv, vminr, rmini;


       real table_Tg_to_BC[34][13] = {
         {-5.006, -4.855, -4.703, -4.551, -4.400, -4.248, 
          -4.096, -3.945, -3.793, -3.642, -3.490, -3.339, -3.187},
         {-4.024, -3.915, -3.805, -3.696, -3.587, -3.478, 
          -3.369, -3.260, -3.150, -3.041, -2.932, -2.823, -2.714},
         {-2.965, -2.900, -2.834, -2.769, -2.704, -2.639, 
          -2.573, -2.508, -2.443, -2.378, -2.312, -2.247, -2.182},
         {-2.081, -2.051, -2.021, -1.992, -1.962, -1.933, 
          -1.903, -1.873, -1.844, -1.814, -1.785, -1.755, -1.726},
         {-1.378, -1.375, -1.372, -1.370, -1.367, -1.364, 
          -1.361, -1.357, -1.352, -1.348, -1.344, -1.340, -1.336},
         {-.855, -.871, -.887, -.903, -.918, -.934, 
          -.949, -.965, -.980, -.996, -1.012, -1.028, -1.043},
         {-.523, -.544, -.564, -.583, -.600, -.617, 
          -.638, -.668, -.703, -.737, -.768, -.798, -.827},
         {-.331, -.348, -.363, -.376, -.384, -.392, 
          -.410, -.449, -.500, -.546, -.587, -.626, -.661},
         {-.019, -.043, -.064, -.083, -.098, -.112, 
          -.135, -.178, -.232, -.281, -.321, -.356, -.384},
         {.238, .199, .159, .121, .082, .045, 
          .007, -.036, -.083, -.125, -.157, -.182, -.199},
         {.369, .325, .277, .229, .177, .125, 
          .077, .032, -.011, -.050, -.079, -.101, -.115},
         {.353, .320, .283, .242, .197, .150, 
          .104, .060, .017, -.022, -.056, -.085, -.108},
         {.297, .272, .245, .216, .186, .151, 
          .114, .074, .033, -.006, -.043, -.079, -.112},
         {.234, .218, .201, .182, .163, .139, 
          .112, .079, .043, .004, -.033, -.071, -.109},
         {.093, .113, .123, .126, .118, .104, 
          .085, .062, .034, .002, -.032, -.067, -.105},
         {-.144, -.062, -.005, .031, .038, .033, 
          .024, .011, -.006, -.027, -.052, -.080, -.112},
         {-.380, -.249, -.156, -.092, -.071, -.068, 
          -.070, -.074, -.081, -.091, -.104, -.120, -.140},
         {-.572, -.414, -.299, -.220, -.189, -.181, 
          -.179, -.178, -.178, -.179, -.183, -.188, -.194},
         {-.728, -.554, -.426, -.336, -.299, -.288, 
          -.285, -.283, -.281, -.279, -.278, -.277, -.276},
         {-.979, -.786, -.644, -.544, -.501, -.487, 
          -.485, -.484, -.483, -.482, -.481, -.480, -.479},
         {-1.196, -.995, -.847, -.742, -.695, -.680, 
          -.678, -.679, -.680, -.681, -.683, -.684, -.687},
         {-1.398, -1.197, -1.048, -.943, -.896, -.881, 
          -.879, -.881, -.883, -.885, -.887, -.890, -.893},
         {-1.602, -1.405, -1.260, -1.159, -1.115, -1.103, 
          -1.104, -1.107, -1.109, -1.111, -1.114, -1.117, -1.121},
         {-1.831, -1.631, -1.483, -1.381, -1.338, -1.328, 
          -1.333, -1.340, -1.343, -1.346, -1.349, -1.352, -1.356},
         {-2.022, -1.815, -1.663, -1.557, -1.513, -1.505, 
          -1.512, -1.521, -1.527, -1.530, -1.533, -1.535, -1.537},
         {-2.340, -2.112, -1.945, -1.830, -1.783, -1.777, 
          -1.791, -1.806, -1.816, -1.821, -1.824, -1.824, -1.822},
         {-2.605, -2.345, -2.153, -2.022, -2.020, -2.016, 
          -2.024, -2.050, -2.070, -2.083, -2.084, -2.084, -2.083},
         {-2.886, -2.608, -2.402, -2.284, -2.284, -2.284, 
          -2.284, -2.322, -2.353, -2.375, -2.375, -2.375, -2.375},
         {-3.124, -2.846, -2.640, -2.522, -2.522, -2.522, 
          -2.522, -2.567, -2.599, -2.628, -2.648, -2.648, -2.648},
         {-3.566, -3.288, -3.082, -2.964, -2.964, -2.964, 
          -2.964, -2.964, -3.010, -3.048, -3.070, -3.070, -3.070},
         {-3.978, -3.700, -3.494, -3.376, -3.376, -3.376, 
          -3.376, -3.376, -3.361, -3.382, -3.404, -3.404, -3.404},
         {-4.366, -4.088, -3.882, -3.764, -3.764, -3.764, 
          -3.764, -3.764, -3.764, -3.752, -3.748, -3.748, -3.748},
         {-4.732, -4.454, -4.248, -4.130, -4.130, -4.130, 
          -4.130, -4.130, -4.130, -4.130, -4.124, -4.124, -4.124},
         {-5.076, -4.789, -4.592, -4.474, -4.474, -4.474, 
          -4.474, -4.474, -4.474, -4.474, -4.472, -4.472, -4.472}
      };
       real table_Tg_to_UminB[34][13] = {
         {4.680, 4.362, 4.044, 3.725, 3.407, 3.089, 
          2.771, 2.453, 2.134, 1.816, 1.498, 1.180, .862},
         {4.156, 3.874, 3.593, 3.312, 3.031, 2.749, 
          2.468, 2.187, 1.906, 1.624, 1.343, 1.062, .781},
         {3.535, 3.304, 3.073, 2.842, 2.611, 2.381, 
          2.150, 1.919, 1.688, 1.457, 1.226, .995, .764},
         {2.934, 2.756, 2.578, 2.399, 2.221, 2.043, 
          1.865, 1.687, 1.509, 1.331, 1.153, .975, .796},
         {2.357, 2.241, 2.124, 2.006, 1.887, 1.768, 
          1.647, 1.527, 1.406, 1.285, 1.162, 1.039, .915},
         {1.782, 1.723, 1.665, 1.606, 1.547, 1.487, 
          1.428, 1.368, 1.308, 1.248, 1.187, 1.127, 1.066},
         {1.324, 1.281, 1.245, 1.216, 1.196, 1.184, 
          1.175, 1.168, 1.164, 1.168, 1.179, 1.198, 1.223},
         {1.062, .991, .939, .905, .892, .898, 
          .915, .935, .966, 1.018, 1.088, 1.176, 1.282},
         {.774, .680, .607, .554, .522, .512, 
          .518, .535, .568, .630, .715, .824, .956},
         {.583, .508, .441, .384, .334, .297, 
          .274, .264, .272, .301, .350, .420, .510},
         {.440, .394, .345, .295, .239, .187, 
          .142, .108, .087, .081, .093, .121, .165},
         {.309, .306, .289, .261, .217, .167, 
          .114, .063, .019, -.017, -.043, -.059, -.065},
         {.190, .237, .263, .269, .252, .216, 
          .167, .110, .050, -.009, -.066, -.122, -.177},
         {.039, .134, .202, .246, .262, .254, 
          .224, .175, .114, .045, -.032, -.116, -.209},
         {-.222, -.063, .058, .146, .194, .216, 
          .215, .191, .146, .082, -.002, -.104, -.226},
         {-.571, -.325, -.137, .002, .074, .117, 
          .142, .147, .128, .084, .013, -.083, -.206},
         {-.871, -.554, -.314, -.136, -.046, .008, 
          .046, .070, .075, .059, .024, -.029, -.101},
         {-1.075, -.723, -.454, -.253, -.149, -.087, 
          -.042, -.010, .009, .013, .012, .000, -.019},
         {-1.231, -.857, -.570, -.355, -.241, -.174, 
          -.125, -.088, -.061, -.045, -.033, -.027, -.027},
         {-1.449, -1.052, -.748, -.520, -.396, -.324, 
          -.273, -.231, -.198, -.173, -.155, -.143, -.139},
         {-1.600, -1.194, -.881, -.647, -.520, -.446, 
          -.396, -.356, -.323, -.297, -.277, -.265, -.259},
         {-1.743, -1.322, -.999, -.757, -.624, -.548, 
          -.498, -.461, -.430, -.407, -.389, -.377, -.371},
         {-1.922, -1.472, -1.124, -.863, -.718, -.636, 
          -.585, -.549, -.522, -.501, -.486, -.477, -.475},
         {-2.147, -1.649, -1.263, -.973, -.807, -.713, 
          -.660, -.625, -.599, -.581, -.568, -.562, -.563},
         {-2.331, -1.796, -1.379, -1.066, -.884, -.781, 
          -.725, -.691, -.666, -.649, -.638, -.633, -.636},
         {-2.695, -2.078, -1.595, -1.231, -1.014, -.890, 
          -.826, -.791, -.768, -.752, -.743, -.741, -.748},
         {-3.028, -2.329, -1.777, -1.360, -1.105, -.958, 
          -.904, -.859, -.833, -.818, -.809, -.804, -.813},
         {-3.400, -2.610, -1.980, -1.504, -1.207, -1.034, 
          -.968, -.921, -.893, -.878, -.869, -.863, -.871},
         {-3.733, -2.861, -2.162, -1.633, -1.298, -1.102, 
          -1.023, -.978, -.948, -.931, -.923, -.918, -.921},
         {-4.310, -3.295, -2.477, -1.856, -1.455, -1.220, 
          -1.083, -1.070, -1.036, -1.021, -1.012, -1.010, -1.010},
         {-4.797, -3.662, -2.744, -2.045, -1.588, -1.319, 
          -1.122, -1.122, -1.102, -1.086, -1.076, -1.076, -1.076},
         {-5.219, -3.980, -2.974, -2.209, -1.704, -1.405, 
          -1.131, -1.131, -1.131, -1.124, -1.120, -1.120, -1.120},
         {-5.591, -4.261, -3.178, -2.353, -1.805, -1.481, 
          -1.141, -1.141, -1.141, -1.141, -1.139, -1.139, -1.139},
         {-5.924, -4.512, -3.360, -2.482, -1.896, -1.549, 
          -1.153, -1.153, -1.153, -1.153, -1.151, -1.151, -1.151},
      };
       real table_Tg_to_BminV[34][13] = {
         {3.126, 2.999, 2.871, 2.744, 2.617, 2.490, 
          2.363, 2.236, 2.108, 1.981, 1.854, 1.727, 1.600},
         {2.785, 2.679, 2.573, 2.466, 2.360, 2.254, 
          2.147, 2.041, 1.934, 1.828, 1.722, 1.615, 1.509},
         {2.395, 2.314, 2.233, 2.153, 2.073, 1.992, 
          1.912, 1.831, 1.751, 1.670, 1.590, 1.509, 1.428},
         {2.041, 1.984, 1.927, 1.870, 1.813, 1.757, 
          1.700, 1.643, 1.586, 1.529, 1.472, 1.415, 1.358},
         {1.704, 1.677, 1.649, 1.620, 1.589, 1.558, 
          1.526, 1.493, 1.459, 1.424, 1.390, 1.356, 1.321},
         {1.436, 1.423, 1.411, 1.398, 1.385, 1.372, 
          1.359, 1.345, 1.331, 1.317, 1.303, 1.289, 1.274},
         {1.380, 1.324, 1.278, 1.242, 1.219, 1.201, 
          1.192, 1.193, 1.198, 1.206, 1.214, 1.225, 1.237},
         {1.468, 1.336, 1.230, 1.149, 1.097, 1.061, 
          1.047, 1.056, 1.078, 1.104, 1.134, 1.169, 1.209},
         {1.294, 1.150, 1.035, .946, .888, .850, 
          .838, .853, .883, .919, .963, 1.015, 1.076},
         {.839, .781, .733, .697, .670, .657, 
          .660, .680, .708, .741, .784, .834, .893},
         {.398, .422, .440, .455, .462, .473, 
          .491, .516, .545, .577, .615, .659, .708},
         {.111, .170, .220, .263, .296, .326, 
          .356, .385, .415, .445, .477, .510, .544},
         {.046, .067, .095, .128, .171, .214, 
          .252, .287, .318, .347, .376, .403, .430},
         {.122, .064, .037, .033, .069, .111, 
          .155, .197, .235, .268, .301, .333, .364},
         {.253, .114, .027, -.020, -.004, .029, 
          .069, .113, .158, .197, .238, .278, .319},
         {.380, .182, .048, -.035, -.042, -.023, 
          .007, .045, .088, .130, .176, .225, .276},
         {.450, .227, .070, -.032, -.056, -.051, 
          -.033, -.004, .031, .069, .116, .171, .234},
         {.461, .236, .076, -.032, -.065, -.070, 
          -.061, -.041, -.014, .018, .065, .124, .196},
         {.451, .229, .070, -.039, -.075, -.085, 
          -.082, -.068, -.047, -.019, .028, .088, .165},
         {.425, .207, .050, -.059, -.096, -.110, 
          -.112, -.105, -.091, -.068, -.027, .027, .098},
         {.412, .192, .033, -.077, -.116, -.130, 
          -.134, -.129, -.119, -.101, -.070, -.029, .025},
         {.403, .183, .023, -.091, -.132, -.148, 
          -.152, -.148, -.139, -.124, -.100, -.069, -.029},
         {.395, .175, .015, -.100, -.145, -.164, 
          -.169, -.165, -.156, -.142, -.122, -.096, -.063},
         {.387, .167, .007, -.108, -.155, -.178, 
          -.184, -.181, -.172, -.159, -.140, -.117, -.088},
         {.380, .160, .000, -.115, -.163, -.190, 
          -.197, -.194, -.185, -.173, -.156, -.136, -.111},
         {.367, .147, -.013, -.128, -.176, -.206, 
          -.217, -.214, -.206, -.194, -.180, -.162, -.141},
         {.355, .135, -.025, -.140, -.188, -.214, 
          -.228, -.226, -.218, -.206, -.191, -.173, -.151},
         {.342, .122, -.038, -.153, -.201, -.235, 
          -.235, -.241, -.238, -.231, -.203, -.185, -.162},
         {.330, .110, -.050, -.165, -.213, -.248, 
          -.248, -.257, -.255, -.250, -.213, -.195, -.172},
         {.310, .090, -.070, -.185, -.233, -.281, 
          -.281, -.281, -.286, -.282, -.247, -.229, -.206},
         {.293, .073, -.087, -.202, -.250, -.281, 
          -.281, -.281, -.298, -.304, -.274, -.256, -.233},
         {.278, .058, -.102, -.217, -.265, -.305, 
          -.305, -.305, -.305, -.311, -.284, -.266, -.243},
         {.265, .045, -.115, -.230, -.278, -.318, 
          -.318, -.318, -.318, -.318, -.292, -.274, -.251},
         {.253, .033, -.127, -.242, -.290, -.324, 
          -.324, -.324, -.324, -.324, -.298, -.280, -.257},
      };

       real table_Tg_to_VminR[34][13] = {
         {2.050, 1.999, 1.949, 1.898, 1.847, 1.796, 
          1.746, 1.695, 1.645, 1.594, 1.543, 1.493, 1.442},
         {1.752, 1.712, 1.671, 1.630, 1.590, 1.549, 
          1.508, 1.467, 1.427, 1.386, 1.345, 1.305, 1.264},
         {1.424, 1.397, 1.370, 1.343, 1.317, 1.289, 
          1.262, 1.235, 1.209, 1.182, 1.155, 1.128, 1.101},
         {1.138, 1.124, 1.111, 1.097, 1.084, 1.070, 
          1.056, 1.042, 1.029, 1.016, 1.002, .988, .975},
         {.895, .897, .900, .902, .904, .906, 
          .907, .908, .909, .910, .911, .913, .915},
         {.694, .708, .723, .738, .752, .767, 
          .781, .795, .810, .824, .838, .853, .867},
         {.570, .582, .596, .611, .627, .645, 
          .665, .689, .715, .740, .761, .781, .799},
         {.514, .513, .516, .523, .533, .546, 
          .568, .596, .628, .659, .682, .701, .714},
         {.395, .391, .391, .395, .402, .413, 
          .432, .458, .487, .517, .538, .556, .567},
         {.259, .269, .280, .293, .307, .322, 
          .341, .361, .383, .407, .428, .447, .464},
         {.149, .170, .191, .211, .231, .251, 
          .270, .288, .306, .326, .346, .367, .388},
         {.075, .099, .122, .145, .167, .188, 
          .208, .226, .244, .262, .280, .299, .317},
         {.033, .049, .067, .085, .107, .128, 
          .149, .169, .189, .206, .223, .237, .251},
         {.040, .032, .033, .040, .057, .076, 
          .096, .117, .137, .156, .174, .191, .207},
         {.112, .063, .033, .018, .024, .038, 
          .054, .072, .090, .110, .132, .155, .179},
         {.227, .128, .059, .016, .009, .015, 
          .026, .037, .051, .068, .088, .111, .137},
         {.314, .181, .087, .024, .006, .004, 
          .008, .014, .022, .032, .045, .061, .079},
         {.351, .207, .102, .030, .005, -.002, 
          -.003, -.001, .002, .007, .014, .023, .034},
         {.369, .218, .108, .032, .003, -.008, 
          -.012, -.013, -.012, -.010, -.007, -.002, .005},
         {.381, .223, .108, .027, -.006, -.019, 
          -.025, -.028, -.030, -.031, -.031, -.030, -.028},
         {.367, .211, .097, .017, -.015, -.028, 
          -.034, -.038, -.041, -.043, -.043, -.043, -.041},
         {.354, .200, .087, .007, -.024, -.037, 
          -.043, -.047, -.050, -.052, -.052, -.052, -.050},
         {.354, .197, .082, .000, -.032, -.046, 
          -.052, -.056, -.058, -.060, -.061, -.062, -.063},
         {.375, .208, .085, -.003, -.039, -.054, 
          -.060, -.064, -.066, -.067, -.069, -.070, -.071},
         {.395, .219, .089, -.004, -.044, -.061, 
          -.067, -.070, -.072, -.073, -.075, -.077, -.079},
         {.452, .254, .105, -.001, -.049, -.071, 
          -.078, -.080, -.081, -.082, -.083, -.084, -.085},
         {.532, .305, .134, .011, -.049, -.076, 
          -.085, -.086, -.087, -.087, -.086, -.085, -.083}, // last valid value
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
         {-0.83, -0.83, -0.83, -0.83, -0.83, -0.83, 
          -0.83, -0.83, -0.83, -0.83, -0.83, -0.83, -0.83},
     };
#if 0
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
     };
#endif
       real table_Tg_to_RminI[34][13] = {
         {3.046, 2.939, 2.831, 2.723, 2.615, 2.508, 
          2.400, 2.293, 2.185, 2.077, 1.969, 1.861, 1.754},
         {2.460, 2.388, 2.315, 2.242, 2.169, 2.097, 
          2.024, 1.951, 1.879, 1.806, 1.733, 1.660, 1.587},
         {1.835, 1.795, 1.757, 1.718, 1.679, 1.640, 
          1.601, 1.562, 1.523, 1.484, 1.445, 1.406, 1.367},
         {1.325, 1.312, 1.298, 1.285, 1.272, 1.259, 
          1.245, 1.232, 1.219, 1.206, 1.193, 1.179, 1.166},
         {.932, .934, .936, .937, .939, .941, 
          .943, .945, .947, .948, .950, .952, .954},
         {.665, .676, .687, .697, .708, .719, 
          .730, .741, .752, .762, .773, .784, .795},
         {.516, .530, .544, .557, .570, .583, 
          .596, .611, .627, .642, .656, .668, .679},
         {.448, .461, .472, .483, .493, .503, 
          .515, .530, .546, .562, .573, .582, .587},
         {.368, .376, .384, .392, .399, .407, 
          .415, .426, .439, .451, .459, .465, .467},
         {.300, .306, .313, .320, .328, .337, 
          .345, .354, .364, .374, .381, .387, .391},
         {.243, .245, .250, .256, .265, .275, 
          .284, .293, .302, .310, .318, .324, .329},
         {.196, .195, .196, .200, .206, .215, 
          .224, .235, .245, .255, .264, .273, .282},
         {.153, .154, .155, .157, .159, .163, 
          .171, .181, .192, .203, .217, .233, .250},
         {.114, .119, .122, .124, .123, .123, 
          .127, .134, .143, .155, .171, .190, .213},
         {.087, .094, .097, .098, .095, .092, 
          .092, .096, .101, .110, .123, .139, .160},
         {.080, .080, .078, .076, .071, .067, 
          .064, .064, .066, .070, .077, .086, .099},
         {.081, .071, .062, .054, .049, .044, 
          .040, .036, .035, .034, .036, .039, .043},
         {.073, .056, .043, .033, .027, .022, 
          .018, .013, .009, .005, .002, -.001, -.004},
         {.062, .042, .027, .015, .008, .004, 
          .000, -.005, -.010, -.015, -.020, -.025, -.031},
         {.051, .025, .005, -.010, -.018, -.022, 
          -.026, -.031, -.035, -.040, -.044, -.048, -.053},
         {.050, .017, -.008, -.026, -.037, -.042, 
          -.046, -.050, -.054, -.058, -.061, -.065, -.068},
         {.045, .009, -.020, -.041, -.053, -.059, 
          -.063, -.066, -.070, -.073, -.077, -.080, -.084},
         {.035, -.003, -.033, -.055, -.068, -.074, 
          -.077, -.080, -.084, -.086, -.089, -.092, -.095},
         {.033, -.010, -.043, -.067, -.081, -.088, 
          -.090, -.093, -.096, -.098, -.101, -.104, -.107},
         {.043, -.006, -.045, -.074, -.091, -.099, 
          -.101, -.103, -.106, -.108, -.111, -.113, -.115},
         {.084, .014, -.040, -.081, -.105, -.116, 
          -.119, -.120, -.121, -.123, -.125, -.128, -.130},
         {.161, .062, -.016, -.074, -.110, -.126, 
          -.129, -.129, -.130, -.131, -.133, -.135, -.137}, // last valid value
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
         {-.137, -.137, -.137, -.137, -.137, -.137, 
          -.137, -.137, -.137, -.137, -.137, -.137, -.137},
     };
#if 0
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
         {9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 
          9.999, 9.999, 9.999, 9.999, 9.999, 9.999, 9.999},
     };
#endif
	 
       real table_tgr[34] = {2800.,3000.,3250.,3500.,3750.,4000.,4250.,4500.,
              5000.,5500.,6000.,6500.,7000.,7500.,8000.,8500.,9000.,
              9500.,10000.,11000.,12000.,13000.,14000.,15000.,16000.,
              18000.,20000.,22500.,25000.,30000.,35000.,40000.,
                45000.,50000.};

  // modify static tgr table to log10
  for(int i=0; i<34; i++) 
      table_tgr[i] = log10(table_tgr[i]);

  real ggr[13] = {0.0,0.5,1.0,1.5,2.0,2.5,3.0,3.5,4.0,4.5,5.0,5.5,6.0};

     //     real ***table = allocate_terrible_table();

     //     real* Tg_to_BC = table_Tg_to_BC();
     //     real* Tg_to_UminB = table_Tg_to_UminB();
     //     real* Tg_to_BminV = table_Tg_to_BminV();

    real gconst=-10.6071;

      if (logl==0 || logt==0) {
	mv=20;

	bminv=0;
	uminb=0;
	return;
      }
      
      real logm = log10(mass);
      real logg = logm + 4*logt - logl + gconst;

      int int1, ing1 = 2*dynamic_cast(int, rint(logg)) - 1;
      int int2, ing2 = ing1+1;
      
      if (ing1 < 1) {
	ing1 = 0;
	ing2 = 1;
      }
      if (ing2>12) {
	ing1 = 11;
	ing2 = 12;
      }
      real logg1 =ggr[ing1];
      real logg2 =ggr[ing2]; // values of log g to interpolate between

      if (logt > table_tgr[33]) {
	int1 = 32;
	int2 = 33;
      } 
      else if (logt <= table_tgr[0]) {
	int1 = 0;
	int2 = 1;
      }
      else
	for (int i=0; i<34; i++) {
	  if (logt <= table_tgr[i]) {
            int1 = i-1;
	    int2 = i;
            break;
	  }
	}
      
// Compute which two lines from the file ubvri.sol correspond with the two
// logT-logg pairs.

      real logt1 = table_tgr[int1];
      real logt2 = table_tgr[int2];
      
      int k0, k1, k2;
      for (int k=0; k<2; k++) 
	for (int l=0; l<2; l++) {
	  
	  k0 = 2*k + l;
	  k1 = int1 + k;
          k2 = ing1 + l;
	  bc[k0] = table_Tg_to_BC[k1][k2];
	  ub[k0] = table_Tg_to_UminB[k1][k2];
	  bv[k0] = table_Tg_to_BminV[k1][k2];
	  vr[k0] = table_Tg_to_VminR[k1][k2];
	  ri[k0] = table_Tg_to_RminI[k1][k2];
	  //	  bc[k0] = table[k1][k2][1];
	  //	  ub[k0] = table[k1][k2][2];
	  //	  bv[k0] = table[k1][k2][3];
	}

      real fracg = (logg2-logg)/(logg2-logg1);
      real bc1 = bc[1] - fracg * (bc[1]-bc[0]);
      real ub1 = ub[1] - fracg * (ub[1]-ub[0]);
      real bv1 = bv[1] - fracg * (bv[1]-bv[0]);
      real vr1 = vr[1] - fracg * (vr[1]-vr[0]);
      real ri1 = ri[1] - fracg * (ri[1]-ri[0]);
      real bc2 = bc[3] - fracg * (bc[3]-bc[2]);
      real ub2 = ub[3] - fracg * (ub[3]-ub[2]);
      real bv2 = bv[3] - fracg * (bv[3]-bv[2]);
      real vr2 = vr[3] - fracg * (vr[3]-vr[2]);
      real ri2 = ri[3] - fracg * (ri[3]-ri[2]);

      real fract = (logt2-logt)/(logt2-logt1);
      real bcx = bc2 - fract * (bc2-bc1);
      real ubx = ub2 - fract * (ub2-ub1);
      real bvx = bv2 - fract * (bv2-bv1);
      real vrx = vr2 - fract * (vr2-vr1);
      real rix = ri2 - fract * (ri2-ri1);

      real mbol = 4.75 - 2.5 * logl;
      mv = mbol - bcx;
      uminb = ubx;
      bminv = bvx;
      vminr = vrx;
      rmini = rix;

      V = mv;
      B = V + bminv;
      U = B + uminb;
      R = V - vminr;
      I = R - rmini;

}

  
void combine_ubvri(real Up, real Bp, real Vp, real Rp, real Ip,
		   real Us, real Bs, real Vs, real Rs, real Is,
		   real &U, real &B, real &V, real &R, real &I) {
  
    real ln10g25 = 2.3025851/2.5;

    U = -2.5 * log(exp(-ln10g25*Up) + exp(-ln10g25*Us));
    B = -2.5 * log(exp(-ln10g25*Bp) + exp(-ln10g25*Bs));
    V = -2.5 * log(exp(-ln10g25*Vp) + exp(-ln10g25*Vs));
    R = -2.5 * log(exp(-ln10g25*Rp) + exp(-ln10g25*Rs));
    I = -2.5 * log(exp(-ln10g25*Ip) + exp(-ln10g25*Is));

}

void ltm_to_ubvri(const real logl_prim,
		  const real logt_prim,
		  const real mass_prim,
		  const real logl_sec,
		  const real logt_sec,
		  const real mass_sec,
		  real& U,
		  real& B,
		  real& V,
		  real& R,
		  real& I) {

     real Up, Bp, Vp, Rp, Ip;
     ltm_to_ubvri(logl_prim, logt_prim, mass_prim,
		  Up, Bp, Vp, Rp, Ip);
     real Us, Bs, Vs, Rs, Is;
     ltm_to_ubvri(logl_sec, logt_sec, mass_sec,
		  Us, Bs, Vs, Rs, Is);

     combine_ubvri(Up, Bp, Vp, Rp, Ip,
		   Us, Bs, Vs, Rs, Is,
		   U,  B,  V,  R,  I); 
}


#ifdef HAVE_EXP10
#error we would like to get this autodetected via configure.in, and not use myexp10, but use exp10
#else

local inline real myexp10(real x)	// sorry Simon -- no such function!  (Steve)
{
    return pow(10.0, x);
}
#endif

void get_Lubvri_star(dyn *bi, stellar_type& stype,
		     real& Lu, real& Lb, real& Lv, real& Lr, real& Li) {

    real u, b, v, r, i;
    u = b = v = r = i = VERY_LARGE_NUMBER;

    get_ubvri_star(bi, stype, u, b, v, r, i);

    real ln10g25 = 2.3025851/2.5;
    Lu = myexp10(-ln10g25*u);
    Lb = myexp10(-ln10g25*b);
    Lv = myexp10(-ln10g25*v);
    Lr = myexp10(-ln10g25*r);
    Li = myexp10(-ln10g25*i);
}

void get_ubvri_star(dyn *bi, stellar_type& stype,
		    real& U, real& B, real& V, real& R, real& I) {

//     real mass = bi->get_starbase()->conv_m_dyn_to_star(bi->get_mass());

     star_type_spec tpe_class = NAC;
     spectral_class star_class;
     real mass=0, T_rel=0, T_eff=0, R_eff=0, L_eff=0;
     if (bi->get_use_sstar()) {
       	stype = bi->get_starbase()->get_element_type();
        T_rel = bi->get_starbase()->get_relative_age();
        mass = bi->get_starbase()->get_total_mass();
        T_eff = bi->get_starbase()->temperature();
        L_eff = bi->get_starbase()->get_luminosity();
        star_class = get_spectral_class(T_eff);
	R_eff = bi->get_starbase()->get_effective_radius();
     }
     else if (find_qmatch(bi->get_star_story(), "T_eff")) {
       stype = extract_stellar_type_string(
	       getsq(bi->get_star_story(), "Type")); 
       T_rel = getrq(bi->get_star_story(), "T_rel");
       mass = getrq(bi->get_star_story(), "M_env")
            + getrq(bi->get_star_story(), "M_core");
       T_eff = getrq(bi->get_star_story(), "T_eff");
       star_class = get_spectral_class(T_eff);
       L_eff = getrq(bi->get_star_story(), "L_eff");
       if (find_qmatch(bi->get_star_story(), "Class"))
	 tpe_class = extract_stellar_spec_summary_string(
             getsq(bi->get_star_story(), "Class"));
       if (L_eff>0)
          R_eff = pow(T_eff/cnsts.parameters(solar_temperature), 2)
	       / sqrt(L_eff);
     }
     else {
       cerr << "    No stellar information found for: ";
       bi->pretty_print_node(cout);

       return;
     }

     real log_Leff = log10(L_eff);
     real log_Teff = log10(T_eff);
     ltm_to_ubvri(log_Leff, log_Teff, mass,
		  U, B, V, R, I);

//     if(V>B) {
//       cerr << type_string(stype) << " " << type_string(star_class)<<" ";
//       PRC(T_rel);PRC(mass);PRC(T_eff);PRC(L_eff);PRL(R_eff);
//     }
}

local void print_star(stellar_type stype, int nstar, real mass, vec pos, 
		 real U, real B, real V, real R, real I) {

     cerr << stype <<" "<< nstar<<" "<<mass << " " << pos << " " 
	  << U <<" "<< B <<" "<< V <<" "<< R <<" "<< I << endl;

}

local void print_ubvri_star(dyn *bi, vec pos) {

//     real mass = bi->get_starbase()->conv_m_dyn_to_star(bi->get_mass());
     real time = bi->get_starbase()->conv_t_dyn_to_star(bi->get_system_time());

     real rcom = abs(pos);
     rcom = bi->get_starbase()->conv_r_dyn_to_star(rcom);
     // And now to parsec
     real Rsun_per_parsec = cnsts.parameters(solar_radius)
                          / cnsts.parameters(parsec);
     real Rdyn_to_parsec = Rsun_per_parsec*
                           bi->get_starbase()->conv_r_dyn_to_star(1);
                       
     rcom *= Rsun_per_parsec;
     pos  *= Rdyn_to_parsec;
	  
     star_type_spec tpe_class = NAC;
     spectral_class star_class;
     stellar_type stype = NAS;
     real mass=0, T_rel=0, T_eff=0, R_eff=0, L_eff=0;
     if (bi->get_use_sstar()) {
       	stype = bi->get_starbase()->get_element_type();
        T_rel = bi->get_starbase()->get_relative_age();
        mass = bi->get_starbase()->get_total_mass();
        T_eff = bi->get_starbase()->temperature();
        L_eff = bi->get_starbase()->get_luminosity();
        star_class = get_spectral_class(T_eff);
	R_eff = bi->get_starbase()->get_effective_radius();
     }
     else if (find_qmatch(bi->get_star_story(), "T_eff")) {
       stype = extract_stellar_type_string(
	       getsq(bi->get_star_story(), "Type")); 
       T_rel = getrq(bi->get_star_story(), "T_rel");
       mass = getrq(bi->get_star_story(), "M_env");
            + getrq(bi->get_star_story(), "M_core");
       T_eff = getrq(bi->get_star_story(), "T_eff");
       star_class = get_spectral_class(T_eff);
       L_eff = getrq(bi->get_star_story(), "L_eff");
       if (find_qmatch(bi->get_star_story(), "Class"))
	 tpe_class = extract_stellar_spec_summary_string(
             getsq(bi->get_star_story(), "Class"));
       if (L_eff>0)
          R_eff = pow(T_eff/cnsts.parameters(solar_temperature), 2)
 	       / sqrt(L_eff);
     }
     else {
       cerr << "    No stellar information found for: ";
       bi->pretty_print_node(cout);
       return;
     }

     real log_Leff = log10(L_eff);
     real log_Teff = log10(T_eff);
     real U, B, V, R, I;
     ltm_to_ubvri(log_Leff, log_Teff, mass,
		  U, B, V, R, I);

     print_star(stype, 0, mass, pos, U, B, V, R, I);

}

local void print_ubvri_binary(dyn *bi, vec dc_pos) {


  vec com = bi->get_pos() - dc_pos;
  real rcom = abs(com);
  rcom = bi->get_starbase()->conv_r_dyn_to_star(rcom);
  // And now to parsec
  real Rsun_per_parsec = cnsts.parameters(solar_radius)
                       / cnsts.parameters(parsec);
  real Rdyn_to_parsec = Rsun_per_parsec*
                        bi->get_starbase()->conv_r_dyn_to_star(1);
  rcom *= Rsun_per_parsec;
  com  *= Rdyn_to_parsec;

  stellar_type ptype;
  real Up, Bp, Vp, Rp, Ip;
  real mp = bi->get_starbase()
              ->conv_m_dyn_to_star(bi->get_oldest_daughter()->get_mass());
  get_ubvri_star(bi->get_oldest_daughter(), ptype, Up, Bp, Vp, Rp, Ip);

  stellar_type stype;
  real Us, Bs, Vs, Rs, Is;
  real ms = bi->get_starbase()
              ->conv_m_dyn_to_star(bi->get_oldest_daughter()
				     ->get_binary_sister()->get_mass());
  get_ubvri_star(bi->get_oldest_daughter()->get_binary_sister(),
		 stype, Us, Bs, Vs, Rs, Is);

  real U, B, V, R, I;
  combine_ubvri(Up, Bp, Vp, Rp, Ip,
		Us, Bs, Vs, Rs, Is,
		U, B, V, R, I);

  print_star(Double, 2, mp+ms, com, U, B, V, R, I);
//  cerr << "    ";
//  print_star(ptype, 1, mp, com, Up, Bp, Vp, Rp, Ip);
//  cerr << "    ";
//  print_star(stype, 1, ms, com, Us, Bs, Vs, Rs, Is);

}

local int print_ubvri_binary_recursive(dyn *b, vec dc_pos,
				       real mtot, int ncomp,
				       real &U, real &B, real &V, 
				       real &R, real &I) {

  int nb = 0;
  real Up=U, Bp=B, Vp=V, Rp=R, Ip=I;
  real Us, Bs, Vs, Rs, Is;
       Us= Bs= Vs= Rs= Is=VERY_LARGE_NUMBER;
  if (b->get_oldest_daughter()) {
//      PRL(b->n_leaves()); 

      //cerr << " Multiple : " << b->format_label()<<endl;

      for_all_daughters(dyn, b, bb) {
	Up=U; Bp=B; Vp=V; Rp=R; Ip=I;
	//	cerr << "search furthre for "<< bb->format_label()<<endl;
	if (bb->n_leaves() >= 2) {
	  //cerr << "next recursive call"<<endl;
	  print_ubvri_binary_recursive(bb, dc_pos, mtot, ncomp, 
				       Us, Bs, Vs, Rs, Is);
	}
	else if(bb->is_leaf()) {
	  //cerr << "compute UBVRI for leave "<<bb->format_label()<<endl;
	  stellar_type stype=NAS;
	  real mass = bb->get_starbase()->conv_m_dyn_to_star(bb->get_mass());
	  mtot += mass;
	  ncomp++;
	  nb++;
          get_ubvri_star(bb, stype, Us, Bs, Vs, Rs, Is);
	  //print_star(stype, ncomp, mass, dc_pos, Us, Bs, Vs, Rs, Is);
	}

	combine_ubvri(Up, Bp, Vp, Rp, Ip,
		      Us, Bs, Vs, Rs, Is,
		      U, B, V, R, I);

	//PRC(U);PRC(B);PRC(V);PRC(R);PRL(I);

      }
    }


  vec pos = something_relative_to_root(b, &dyn::get_pos);

  // And now to parsec
  real Rsun_per_parsec = cnsts.parameters(solar_radius)
                       / cnsts.parameters(parsec);
  real Rdyn_to_parsec = Rsun_per_parsec*
                        b->get_starbase()->conv_r_dyn_to_star(1);
                       
  pos = (pos-dc_pos)*Rdyn_to_parsec;
  real rcom = abs(pos);

  print_star(Double, ncomp, mtot, pos, U, B, V, R, I);

//  print_ubvri_binary(b, dc_pos); //, U, B, V, R, I);

  return nb;
}


#if 0


local int print_ubvri_binary_recursive(dyn *b, vec dc_pos,
				       real mtot, int ncomp,
				       real &U, real &B, real &V, 
				       real &R, real &I) {

  int nb = 0;
  if (b->get_oldest_daughter()) {

     print_ubvri_binary(b, dc_pos);


      //cerr << " Multiple : " << b->format_label()<<endl;

     real Us, Bs, Vs, Rs, Is;
      for_all_daughters(dyn, b, bb) {
	if (bb->n_leaves() >= 2) {
	  print_ubvri_binary_recursive(bb, dc_pos, mtot, ncomp, 
				       Us, Bs, Vs, Rs, Is);
	}
      }
   }

  return nb;
}
#endif

void put_ubvri(dyn *b) {

  bool cod = false;

  vec dc_pos = 0;
  bool try_com = false;
  if(abs(dc_pos) == 0) {
      if (find_qmatch(b->get_dyn_story(), "density_center_pos")) {
      
	  if (getrq(b->get_dyn_story(), "density_center_time")
	      != b->get_system_time()) {
	      warning("put_ubvri: neglecting out-of-date density center");
	      try_com = true;
	  } else
	      cod = true;
	  
	  dc_pos = getvq(b->get_dyn_story(), "density_center_pos");
      }
      else if (find_qmatch(b->get_dyn_story(), "lagr_pos")) {
	  
	  dc_pos = getvq(b->get_dyn_story(), "lagr_pos");

      }
      else if (try_com && find_qmatch(b->get_dyn_story(), "com_pos")) {
	
	  dc_pos = getvq(b->get_dyn_story(), "com_pos");
      }
  }

  // For now: put density center in geometric origin
//  dc_pos = 0;
  
  int ns=0, nb=0;
  for_all_daughters(dyn, b, bi) 
    if (bi->is_leaf()) {

      print_ubvri_star(bi, bi->get_pos() - dc_pos);
      ns++;
    }

  for_all_daughters(dyn, b, bi) {
    if (!bi->is_leaf()) {                
      int nstar = 0;
      real mtot=0, U, B, V, R, I; 
      U=B=V=R=I = VERY_LARGE_NUMBER;


      nb += print_ubvri_binary_recursive(bi, dc_pos, mtot, nstar, 
					 U, B, V, R, I);
    }

  }
}

#else

main(int argc, char **argv)
{

    bool v_flag = false;
    bool c_flag = false;
    
    real m_prim;                     // Mass [Msun] of primary star 
    real m_sec;                      // Mass [Msun] of secondary star 
    real T_prim;                     // Temperature [Kelvin] of primary star
    real T_sec;                      // Temperature [Kelvin] of secondary star
    real L_prim;                     // Luminosity [Lsun] of primary star 
    real L_sec;                      // Luminosity [Lsun] of secondary star 
    T_prim = T_sec = L_prim = L_sec = -1;

    real mv_prim;                    // Magnitude of primary star
    real bminv_prim;                 // B-V magnitude of primary star
    real uminb_prim;                 // U-B magnitude of primary star
    real mv_sec;                     // Magnitude of secondary star
    real bminv_sec;                  // B-V magnitude of secondary star
    real uminb_sec;                  // U-B magnitude of secondary star
    real mv_bin;                     // Magnitude of binary
    real bminv_bin;                  // B-V magnitude of binary
    real uminb_bin;                  // U-B magnitude of binary
    real U, B, V, R, I;
    
    char* comment;
    check_help();
    extern char *poptarg;
    int c;
    const char *param_string = "M:m:T:t:L:l:v";

    while ((c = pgetopt(argc, argv, param_string,
		    "$Revision$", _SRC_)) != -1)
	switch(c) {
	    case 'M': m_prim = atof(poptarg);
		      break;
	    case 'm': m_sec = atof(poptarg);
		      break;
	    case 'T': T_prim = atof(poptarg);
		      break;
	    case 't': T_sec = atof(poptarg);
		      break;
	    case 'L': L_prim = atof(poptarg);
		      break;
	    case 'l': L_sec = atof(poptarg);
		      break;
	    case 'c': c_flag = true;
		      comment = poptarg;
	    case 'v': v_flag = true;
		      comment = poptarg;
		      break;
            case '?': params_to_usage(cerr, argv[0], param_string);
		      get_help();
		      exit(1);
	}

    bool apply_to_nbody_system = false;
    if (T_prim<=0 && L_prim<=0)
      apply_to_nbody_system = true;

    if (!apply_to_nbody_system) {
      real logl_prim;
      real logt_prim;
      if (L_prim >0 && T_prim >0) {
	logl_prim = log10(L_prim);
	logt_prim = log10(T_prim);

        ltm_to_ubvri(logl_prim, logt_prim, m_prim,
		     U, B, V, R, I);

	cerr << "Primary star:           "
	     << " Mv = " << V
	     << " B-V = " << B-V
	     << " U-B = " << U-B
	     << endl;
	PRI(10);PRC(U);PRC(B);PRL(V);
	PRI(10);PRC(R);PRL(I);
      }
    
      real logl_sec;
      real logt_sec;
      if (L_sec >0 && T_sec >0) {
	logl_sec = log10(L_sec);
	logt_sec = log10(T_sec);
	ltm_to_ubvri(logl_sec, logt_sec, m_sec,
		     U, B, V, R, I);

	cerr << "Secondary star:         "
	     << " Mv = " << V
	     << " B-V = " << B-V
	     << " U-B = " << U-B
	     << endl;
	PRI(10);PRC(U);PRC(B);PRL(V);
	PRI(10);PRC(R);PRL(I);

	if (L_prim >0 && T_prim >0) {
	  ltm_to_ubvri(logl_prim, logt_prim, m_prim,
		       logl_sec, logt_sec, m_sec,
		       U, B, V, R, I);

	  cerr << "Combined in the binary: "
	       << " Mv = " << V
	       << " B-V = " << B-V
	       << " U-B = " << U-B
	       << endl;
	  PRI(10);PRC(U);PRC(B);PRL(V);
	  PRI(10);PRC(R);PRL(I);
      
	}
      }
    }
    else {

      cerr.precision(STD_PRECISION);

      dyn *b;

      int nsnap = 0;	
      while (b = get_dyn()) {

	cerr << "Time = " << b->get_system_time() 
	     << " ("<<b->get_starbase()->conv_t_dyn_to_star(b->get_system_time())<<" [Myr])" 
	     << endl;
	addstar(b);                            

	b->set_use_sstar(true); 
	// Do not make new stars but make sure the starbase is searched.
	//b->set_use_sstar(false);

        if (c_flag == TRUE)
	  b->log_comment(comment);
	
        b->log_history(argc, argv);

	nsnap ++; 	
	put_ubvri(b);

	if (v_flag)
	  put_dyn(b);
	
	rmtree(b);
      }
    }
}

#endif
