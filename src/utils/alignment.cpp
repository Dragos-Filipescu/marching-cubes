#include <utils/glsl_alignment.hpp>

#include <utility>

using namespace marching_cubes::utils;

static_assert(alignUp<8>(5) == 8);
static_assert(alignUp<8>(8) == 8);
static_assert(alignUp<8>(9) == 16);

static_assert(!GLSLScalar<std::int8_t>);
static_assert(!GLSLScalar<std::int16_t>);
static_assert(GLSLScalar<std::int32_t>);
static_assert(!GLSLScalar<std::int64_t>);

static_assert(!GLSLScalar<std::uint8_t>);
static_assert(!GLSLScalar<std::uint16_t>);
static_assert(GLSLScalar<std::uint32_t>);
static_assert(!GLSLScalar<std::uint64_t>);

static_assert(GLSLScalar<float>);
static_assert(GLSLScalar<double>);
static_assert(!GLSLScalar<long double>);

static_assert(GLSLScalar<bool>);

// vectors of std::int32_t

static_assert(GLSLVector<glm::vec<2, std::int32_t>>);
static_assert(GLSLVector<glm::vec<3, std::int32_t>>);
static_assert(GLSLVector<glm::vec<4, std::int32_t>>);

// vectors of std::uint32_t

static_assert(GLSLVector<glm::vec<2, std::uint32_t>>);
static_assert(GLSLVector<glm::vec<3, std::uint32_t>>);
static_assert(GLSLVector<glm::vec<4, std::uint32_t>>);

// vectors of float

static_assert(GLSLVector<glm::vec<2, float>>);
static_assert(GLSLVector<glm::vec<3, float>>);
static_assert(GLSLVector<glm::vec<4, float>>);

// vectors of double

static_assert(GLSLVector<glm::vec<2, double>>);
static_assert(GLSLVector<glm::vec<3, double>>);
static_assert(GLSLVector<glm::vec<4, double>>);

// vectors of bool

static_assert(GLSLVector<glm::vec<2, bool>>);
static_assert(GLSLVector<glm::vec<3, bool>>);
static_assert(GLSLVector<glm::vec<4, bool>>);

// matrices of std::int32_t

static_assert(GLSLMatrix<glm::mat<2, 2, std::int32_t>>);
static_assert(GLSLMatrix<glm::mat<2, 3, std::int32_t>>);
static_assert(GLSLMatrix<glm::mat<2, 4, std::int32_t>>);

static_assert(GLSLMatrix<glm::mat<3, 2, std::int32_t>>);
static_assert(GLSLMatrix<glm::mat<3, 3, std::int32_t>>);
static_assert(GLSLMatrix<glm::mat<3, 4, std::int32_t>>);

static_assert(GLSLMatrix<glm::mat<4, 2, std::int32_t>>);
static_assert(GLSLMatrix<glm::mat<4, 3, std::int32_t>>);
static_assert(GLSLMatrix<glm::mat<4, 4, std::int32_t>>);

// matrices of std::uint32_t

static_assert(GLSLMatrix<glm::mat<2, 2, std::uint32_t>>);
static_assert(GLSLMatrix<glm::mat<2, 3, std::uint32_t>>);
static_assert(GLSLMatrix<glm::mat<2, 4, std::uint32_t>>);

static_assert(GLSLMatrix<glm::mat<3, 2, std::uint32_t>>);
static_assert(GLSLMatrix<glm::mat<3, 3, std::uint32_t>>);
static_assert(GLSLMatrix<glm::mat<3, 4, std::uint32_t>>);

static_assert(GLSLMatrix<glm::mat<4, 2, std::uint32_t>>);
static_assert(GLSLMatrix<glm::mat<4, 3, std::uint32_t>>);
static_assert(GLSLMatrix<glm::mat<4, 4, std::uint32_t>>);

// matrices of float

static_assert(GLSLMatrix<glm::mat<2, 2, float>>);
static_assert(GLSLMatrix<glm::mat<2, 3, float>>);
static_assert(GLSLMatrix<glm::mat<2, 4, float>>);

static_assert(GLSLMatrix<glm::mat<3, 2, float>>);
static_assert(GLSLMatrix<glm::mat<3, 3, float>>);
static_assert(GLSLMatrix<glm::mat<3, 4, float>>);

static_assert(GLSLMatrix<glm::mat<4, 2, float>>);
static_assert(GLSLMatrix<glm::mat<4, 3, float>>);
static_assert(GLSLMatrix<glm::mat<4, 4, float>>);

// matrices of double

static_assert(GLSLMatrix<glm::mat<2, 2, double>>);
static_assert(GLSLMatrix<glm::mat<2, 3, double>>);
static_assert(GLSLMatrix<glm::mat<2, 4, double>>);

static_assert(GLSLMatrix<glm::mat<3, 2, double>>);
static_assert(GLSLMatrix<glm::mat<3, 3, double>>);
static_assert(GLSLMatrix<glm::mat<3, 4, double>>);

static_assert(GLSLMatrix<glm::mat<4, 2, double>>);
static_assert(GLSLMatrix<glm::mat<4, 3, double>>);
static_assert(GLSLMatrix<glm::mat<4, 4, double>>);

// matrices of bool

static_assert(GLSLMatrix<glm::mat<2, 2, bool>>);
static_assert(GLSLMatrix<glm::mat<2, 3, bool>>);
static_assert(GLSLMatrix<glm::mat<2, 4, bool>>);

static_assert(GLSLMatrix<glm::mat<3, 2, bool>>);
static_assert(GLSLMatrix<glm::mat<3, 3, bool>>);
static_assert(GLSLMatrix<glm::mat<3, 4, bool>>);

static_assert(GLSLMatrix<glm::mat<4, 2, bool>>);
static_assert(GLSLMatrix<glm::mat<4, 3, bool>>);
static_assert(GLSLMatrix<glm::mat<4, 4, bool>>);

// 1D arrays of scalar types

static_assert(GLSLArray<std::int32_t[1]>);
static_assert(GLSLArray<std::uint32_t[1]>);
static_assert(GLSLArray<float[1]>);
static_assert(GLSLArray<double[1]>);
static_assert(GLSLArray<bool[1]>);

static_assert(GLSLArray<std::array<std::int32_t, 1>>);
static_assert(GLSLArray<std::array<std::uint32_t, 1>>);
static_assert(GLSLArray<std::array<float, 1>>);
static_assert(GLSLArray<std::array<double, 1>>);
static_assert(GLSLArray<std::array<bool, 1>>);

// 1D arrays of vector types

static_assert(GLSLArray<glm::vec<2, std::int32_t>[1]>);
static_assert(GLSLArray<glm::vec<2, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::vec<2, float>[1]>);
static_assert(GLSLArray<glm::vec<2, double>[1]>);
static_assert(GLSLArray<glm::vec<2, bool>[1]>);

static_assert(GLSLArray<glm::vec<3, std::int32_t>[1]>);
static_assert(GLSLArray<glm::vec<3, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::vec<3, float>[1]>);
static_assert(GLSLArray<glm::vec<3, double>[1]>);
static_assert(GLSLArray<glm::vec<3, bool>[1]>);

static_assert(GLSLArray<glm::vec<4, std::int32_t>[1]>);
static_assert(GLSLArray<glm::vec<4, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::vec<4, float>[1]>);
static_assert(GLSLArray<glm::vec<4, double>[1]>);
static_assert(GLSLArray<glm::vec<4, bool>[1]>);



static_assert(GLSLArray<std::array<glm::vec<2, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<2, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<2, float>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<2, double>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<2, bool>, 1>>);

static_assert(GLSLArray<std::array<glm::vec<3, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<3, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<3, float>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<3, double>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<3, bool>, 1>>);

static_assert(GLSLArray<std::array<glm::vec<4, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<4, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<4, float>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<4, double>, 1>>);
static_assert(GLSLArray<std::array<glm::vec<4, bool>, 1>>);

// 1D arrays of matrix types

static_assert(GLSLArray<glm::mat<2, 2, std::int32_t>[1]>);
static_assert(GLSLArray<glm::mat<2, 3, std::int32_t>[1]>);
static_assert(GLSLArray<glm::mat<2, 4, std::int32_t>[1]>);

static_assert(GLSLArray<glm::mat<3, 2, std::int32_t>[1]>);
static_assert(GLSLArray<glm::mat<3, 3, std::int32_t>[1]>);
static_assert(GLSLArray<glm::mat<3, 4, std::int32_t>[1]>);

static_assert(GLSLArray<glm::mat<4, 2, std::int32_t>[1]>);
static_assert(GLSLArray<glm::mat<4, 3, std::int32_t>[1]>);
static_assert(GLSLArray<glm::mat<4, 4, std::int32_t>[1]>);



static_assert(GLSLArray<glm::mat<2, 2, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::mat<2, 3, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::mat<2, 4, std::uint32_t>[1]>);

static_assert(GLSLArray<glm::mat<3, 2, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::mat<3, 3, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::mat<3, 4, std::uint32_t>[1]>);

static_assert(GLSLArray<glm::mat<4, 2, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::mat<4, 3, std::uint32_t>[1]>);
static_assert(GLSLArray<glm::mat<4, 4, std::uint32_t>[1]>);



static_assert(GLSLArray<glm::mat<2, 2, float>[1]>);
static_assert(GLSLArray<glm::mat<2, 3, float>[1]>);
static_assert(GLSLArray<glm::mat<2, 4, float>[1]>);

static_assert(GLSLArray<glm::mat<3, 2, float>[1]>);
static_assert(GLSLArray<glm::mat<3, 3, float>[1]>);
static_assert(GLSLArray<glm::mat<3, 4, float>[1]>);

static_assert(GLSLArray<glm::mat<4, 2, float>[1]>);
static_assert(GLSLArray<glm::mat<4, 3, float>[1]>);
static_assert(GLSLArray<glm::mat<4, 4, float>[1]>);



static_assert(GLSLArray<glm::mat<2, 2, double>[1]>);
static_assert(GLSLArray<glm::mat<2, 3, double>[1]>);
static_assert(GLSLArray<glm::mat<2, 4, double>[1]>);

static_assert(GLSLArray<glm::mat<3, 2, double>[1]>);
static_assert(GLSLArray<glm::mat<3, 3, double>[1]>);
static_assert(GLSLArray<glm::mat<3, 4, double>[1]>);

static_assert(GLSLArray<glm::mat<4, 2, double>[1]>);
static_assert(GLSLArray<glm::mat<4, 3, double>[1]>);
static_assert(GLSLArray<glm::mat<4, 4, double>[1]>);



static_assert(GLSLArray<glm::mat<2, 2, bool>[1]>);
static_assert(GLSLArray<glm::mat<2, 3, bool>[1]>);
static_assert(GLSLArray<glm::mat<2, 4, bool>[1]>);

static_assert(GLSLArray<glm::mat<3, 2, bool>[1]>);
static_assert(GLSLArray<glm::mat<3, 3, bool>[1]>);
static_assert(GLSLArray<glm::mat<3, 4, bool>[1]>);

static_assert(GLSLArray<glm::mat<4, 2, bool>[1]>);
static_assert(GLSLArray<glm::mat<4, 3, bool>[1]>);
static_assert(GLSLArray<glm::mat<4, 4, bool>[1]>);



static_assert(GLSLArray<std::array<glm::mat<2, 2, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 3, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 4, std::int32_t>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<3, 2, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 3, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 4, std::int32_t>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<4, 2, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 3, std::int32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 4, std::int32_t>, 1>>);



static_assert(GLSLArray<std::array<glm::mat<2, 2, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 3, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 4, std::uint32_t>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<3, 2, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 3, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 4, std::uint32_t>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<4, 2, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 3, std::uint32_t>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 4, std::uint32_t>, 1>>);



static_assert(GLSLArray<std::array<glm::mat<2, 2, float>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 3, float>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 4, float>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<3, 2, float>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 3, float>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 4, float>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<4, 2, float>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 3, float>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 4, float>, 1>>);



static_assert(GLSLArray<std::array<glm::mat<2, 2, double>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 3, double>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 4, double>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<3, 2, double>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 3, double>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 4, double>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<4, 2, double>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 3, double>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 4, double>, 1>>);



static_assert(GLSLArray<std::array<glm::mat<2, 2, bool>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 3, bool>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<2, 4, bool>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<3, 2, bool>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 3, bool>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<3, 4, bool>, 1>>);

static_assert(GLSLArray<std::array<glm::mat<4, 2, bool>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 3, bool>, 1>>);
static_assert(GLSLArray<std::array<glm::mat<4, 4, bool>, 1>>);


// 2D arrays of scalar types

static_assert(GLSLArray<std::int32_t[1][1]>);
static_assert(GLSLArray<std::uint32_t[1][1]>);
static_assert(GLSLArray<float[1][1]>);
static_assert(GLSLArray<double[1][1]>);
static_assert(GLSLArray<bool[1][1]>);

static_assert(GLSLArray<std::array<std::array<std::int32_t, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<std::uint32_t, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<float, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<double, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<bool, 1>, 1>>);

// 2D arrays of vector types

static_assert(GLSLArray<glm::vec<2, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::vec<2, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::vec<2, float>[1][1]>);
static_assert(GLSLArray<glm::vec<2, double>[1][1]>);
static_assert(GLSLArray<glm::vec<2, bool>[1][1]>);

static_assert(GLSLArray<glm::vec<3, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::vec<3, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::vec<3, float>[1][1]>);
static_assert(GLSLArray<glm::vec<3, double>[1][1]>);
static_assert(GLSLArray<glm::vec<3, bool>[1][1]>);

static_assert(GLSLArray<glm::vec<4, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::vec<4, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::vec<4, float>[1][1]>);
static_assert(GLSLArray<glm::vec<4, double>[1][1]>);
static_assert(GLSLArray<glm::vec<4, bool>[1][1]>);



static_assert(GLSLArray<std::array<std::array<glm::vec<2, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<2, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<2, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<2, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<2, bool>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::vec<3, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<3, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<3, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<3, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<3, bool>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::vec<4, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<4, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<4, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<4, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::vec<4, bool>, 1>, 1>>);

// 2D arrays of matrix types

static_assert(GLSLArray<glm::mat<2, 2, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 3, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 4, std::int32_t>[1][1]>);

static_assert(GLSLArray<glm::mat<3, 2, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 3, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 4, std::int32_t>[1][1]>);

static_assert(GLSLArray<glm::mat<4, 2, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 3, std::int32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 4, std::int32_t>[1][1]>);



static_assert(GLSLArray<glm::mat<2, 2, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 3, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 4, std::uint32_t>[1][1]>);

static_assert(GLSLArray<glm::mat<3, 2, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 3, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 4, std::uint32_t>[1][1]>);

static_assert(GLSLArray<glm::mat<4, 2, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 3, std::uint32_t>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 4, std::uint32_t>[1][1]>);



static_assert(GLSLArray<glm::mat<2, 2, float>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 3, float>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 4, float>[1][1]>);

static_assert(GLSLArray<glm::mat<3, 2, float>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 3, float>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 4, float>[1][1]>);

static_assert(GLSLArray<glm::mat<4, 2, float>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 3, float>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 4, float>[1][1]>);



static_assert(GLSLArray<glm::mat<2, 2, double>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 3, double>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 4, double>[1][1]>);

static_assert(GLSLArray<glm::mat<3, 2, double>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 3, double>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 4, double>[1][1]>);

static_assert(GLSLArray<glm::mat<4, 2, double>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 3, double>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 4, double>[1][1]>);



static_assert(GLSLArray<glm::mat<2, 2, bool>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 3, bool>[1][1]>);
static_assert(GLSLArray<glm::mat<2, 4, bool>[1][1]>);

static_assert(GLSLArray<glm::mat<3, 2, bool>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 3, bool>[1][1]>);
static_assert(GLSLArray<glm::mat<3, 4, bool>[1][1]>);

static_assert(GLSLArray<glm::mat<4, 2, bool>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 3, bool>[1][1]>);
static_assert(GLSLArray<glm::mat<4, 4, bool>[1][1]>);



static_assert(GLSLArray<std::array<std::array<glm::mat<2, 2, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 3, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 4, std::int32_t>, 1>, 1>>);


static_assert(GLSLArray<std::array<std::array<glm::mat<3, 2, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 3, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 4, std::int32_t>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::mat<4, 2, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 3, std::int32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 4, std::int32_t>, 1>, 1>>);



static_assert(GLSLArray<std::array<std::array<glm::mat<2, 2, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 3, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 4, std::uint32_t>, 1>, 1>>);


static_assert(GLSLArray<std::array<std::array<glm::mat<3, 2, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 3, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 4, std::uint32_t>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::mat<4, 2, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 3, std::uint32_t>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 4, std::uint32_t>, 1>, 1>>);



static_assert(GLSLArray<std::array<std::array<glm::mat<2, 2, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 3, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 4, float>, 1>, 1>>);


static_assert(GLSLArray<std::array<std::array<glm::mat<3, 2, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 3, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 4, float>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::mat<4, 2, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 3, float>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 4, float>, 1>, 1>>);



static_assert(GLSLArray<std::array<std::array<glm::mat<2, 2, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 3, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 4, double>, 1>, 1>>);


static_assert(GLSLArray<std::array<std::array<glm::mat<3, 2, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 3, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 4, double>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::mat<4, 2, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 3, double>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 4, double>, 1>, 1>>);



static_assert(GLSLArray<std::array<std::array<glm::mat<2, 2, bool>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 3, bool>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<2, 4, bool>, 1>, 1>>);


static_assert(GLSLArray<std::array<std::array<glm::mat<3, 2, bool>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 3, bool>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<3, 4, bool>, 1>, 1>>);

static_assert(GLSLArray<std::array<std::array<glm::mat<4, 2, bool>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 3, bool>, 1>, 1>>);
static_assert(GLSLArray<std::array<std::array<glm::mat<4, 4, bool>, 1>, 1>>);


//	Scalar alignments

static_assert(GLSLElementAlignment<GLSLLayout::Std140, std::int32_t>::value == sizeof(std::int32_t));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, std::int32_t>::value == sizeof(std::int32_t));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, std::uint32_t>::value == sizeof(std::uint32_t));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, std::uint32_t>::value == sizeof(std::uint32_t));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, float>::value == sizeof(float));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, float>::value == sizeof(float));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, double>::value == sizeof(double));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, double>::value == sizeof(double));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, bool>::value == 4);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, bool>::value == 4);

//	Vector alignments

//	std::int32_t

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, std::int32_t>>::value == 2 * sizeof(std::int32_t));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, std::int32_t>>::value == 2 * sizeof(std::int32_t));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, std::int32_t>>::value == 4 * sizeof(std::int32_t)); // vec3→16
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, std::int32_t>>::value == 4 * sizeof(std::int32_t)); // vec3 special

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, std::int32_t>>::value == 4 * sizeof(std::int32_t));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, std::int32_t>>::value == 4 * sizeof(std::int32_t));

//	std::uint32_t

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, std::uint32_t>>::value == 2 * sizeof(std::uint32_t));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, std::uint32_t>>::value == 2 * sizeof(std::uint32_t));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, std::uint32_t>>::value == 4 * sizeof(std::uint32_t)); // vec3→16
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, std::uint32_t>>::value == 4 * sizeof(std::uint32_t)); // vec3 special

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, std::uint32_t>>::value == 4 * sizeof(std::uint32_t));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, std::uint32_t>>::value == 4 * sizeof(std::uint32_t));

// float

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, float>>::value == 2 * sizeof(float));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, float>>::value == 2 * sizeof(float));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, float>>::value == 4 * sizeof(float)); // vec3→16
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, float>>::value == 4 * sizeof(float)); // vec3 special

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, float>>::value == 4 * sizeof(float));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, float>>::value == 4 * sizeof(float));

// double

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, double>>::value == 2 * sizeof(double));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, double>>::value == 2 * sizeof(double));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, double>>::value == 4 * sizeof(double)); // vec3→16
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, double>>::value == 4 * sizeof(double)); // vec3 special

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, double>>::value == 4 * sizeof(double));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, double>>::value == 4 * sizeof(double));

// bool

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, bool>>::value == 2 * sizeof(bool));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, bool>>::value == 2 * sizeof(bool));

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, bool>>::value == 4 * sizeof(bool)); // vec3→16
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, bool>>::value == 4 * sizeof(bool)); // vec3 special

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, bool>>::value == 4 * sizeof(bool));
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, bool>>::value == 4 * sizeof(bool));

// 3) Matrix alignments (column arrays)

// Nx2 matrices

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<2, 2, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, float>[2]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<2, 2, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, float>[2]>::value);

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<2, 3, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, float>[2]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<2, 3, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, float>[2]>::value);

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<2, 4, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, float>[2]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<2, 4, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, float>[2]>::value);

// Nx3 matrices

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<3, 2, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, float>[3]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<3, 2, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, float>[3]>::value);

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<3, 3, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, float>[3]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<3, 3, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, float>[3]>::value);

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<3, 4, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, float>[3]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<3, 4, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, float>[3]>::value);

// Nx4 matrices

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<4, 2, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<2, float>[4]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<4, 2, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, float>[4]>::value);

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<4, 3, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<3, float>[4]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<4, 3, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<3, float>[4]>::value);

static_assert(GLSLElementAlignment<GLSLLayout::Std140, glm::mat<4, 4, float>>::value == GLSLElementAlignment<GLSLLayout::Std140, glm::vec<4, float>[4]>::value);
static_assert(GLSLElementAlignment<GLSLLayout::Std430, glm::mat<4, 4, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<4, float>[4]>::value);

// array of vec2

static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std140, glm::vec<2, float>[2]>::value == alignUp<16>(2 * sizeof(float)));
static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std430, glm::vec<2, float>[2]>::value == 2 * sizeof(float));

static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std140, std::array<glm::vec<2, float>, 2>>::value == alignUp<16>(2 * sizeof(float)));
static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std430, std::array<glm::vec<2, float>, 2>>::value == 2 * sizeof(float));

// array of vec3

static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std140, glm::vec<3, float>[2]>::value == alignUp<16>(4 * sizeof(float)));
static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std430, glm::vec<3, float>[2]>::value == 3 * sizeof(float));

static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std140, std::array<glm::vec<3, float>, 2>>::value == alignUp<16>(4 * sizeof(float)));
static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std430, std::array<glm::vec<3, float>, 2>>::value == 3 * sizeof(float));

// array of vec4

static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std140, glm::vec<4, float>[2]>::value == alignUp<16>(4 * sizeof(float)));
static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std430, glm::vec<4, float>[2]>::value == 4 * sizeof(float));

static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std140, std::array<glm::vec<4, float>, 2>>::value == alignUp<16>(4 * sizeof(float)));
static_assert(detail::GLSLArrayElementAlignment<GLSLLayout::Std430, std::array<glm::vec<4, float>, 2>>::value == 4 * sizeof(float));



static_assert(GLSLStructMemberAlignment<GLSLLayout::Std430, glm::vec<2, float>>::value == GLSLElementAlignment<GLSLLayout::Std430, glm::vec<2, float>>::value);

static_assert(GLSLStructAlignment<GLSLLayout::Std430,
	glm::vec<2, float>,
	glm::vec<3, float>
>::value == 12);

static_assert(GLSLStructAlignment<GLSLLayout::Std140,
	glm::vec<2, float>,
	glm::vec<3, float>
>::value == 16);

