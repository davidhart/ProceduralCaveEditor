#pragma once
#ifndef _LEAKDETECTOR_H_
#define _LEAKDETECTOR_H_

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>

#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#endif

#endif