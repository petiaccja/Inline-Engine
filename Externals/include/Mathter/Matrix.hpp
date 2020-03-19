//==============================================================================
// This software is distributed under The Unlicense.
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "Matrix/MatrixImpl.hpp"
#include "Matrix/MatrixFunction.hpp"
#include "Matrix/MatrixArithmetic.hpp"
#include "Matrix/MatrixVectorArithmetic.hpp"
#include "Matrix/MatrixCompare.hpp"
#include "Matrix/MatrixCast.hpp"

#include "Decompositions/DecomposeLU.hpp"
#include "Decompositions/DecomposeQR.hpp"
#include "Decompositions/DecomposeSVD.hpp"

#include "Transforms/OrthographicBuilder.hpp"
#include "Transforms/PerspectiveBuilder.hpp"
#include "Transforms/Rotation2DBuilder.hpp"
#include "Transforms/Rotation3DBuilder.hpp"
#include "Transforms/ScaleBuilder.hpp"
#include "Transforms/ShearBuilder.hpp"
#include "Transforms/TranslationBuilder.hpp"
#include "Transforms/ViewBuilder.hpp"

#include "Transforms/IdentityBuilder.hpp"
#include "Transforms/ZeroBuilder.hpp"
