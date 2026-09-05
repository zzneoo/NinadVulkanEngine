#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>

#define IMATH_HALF_NO_LOOKUP_TABLE

#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <openvdb/tools/Interpolation.h>

#include <DirectXTex.h>
#include <DirectXPackedVector.h>


// ============================================================
// Configuration
// ============================================================

// Texture X -> World X
constexpr uint32_t OUTPUT_WIDTH = 512;

// Texture Y -> World Z
constexpr uint32_t OUTPUT_HEIGHT = 512;

// Texture Z -> World Y
constexpr uint32_t OUTPUT_DEPTH = 64;


// ============================================================
// Downsampling configuration
//
// 3 x 3 x 3 = 27 samples per destination voxel.
//
// Separable weights:
//
// 1, 2, 1
//
// The center sample receives more importance.
// ============================================================

constexpr uint32_t SAMPLE_GRID = 3;

constexpr float FILTER_WEIGHTS[SAMPLE_GRID] =
{
    1.0f,
    2.0f,
    1.0f
};


// ============================================================
// Half-float RGBA voxel
// ============================================================

struct VoxelRGBA
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t a;
};


// ============================================================
// Float to half-float
// ============================================================

static uint16_t FloatToHalf(float value)
{
    return DirectX::PackedVector::XMConvertFloatToHalf(value);
}


// ============================================================
// Convert char string to wide string
// ============================================================

static std::wstring ToWideString(const char* text)
{
    if (!text)
    {
        return {};
    }

    std::wstring result;

    while (*text)
    {
        result.push_back(
            static_cast<unsigned char>(*text)
        );

        ++text;
    }

    return result;
}


// ============================================================
// Get active voxel world-space bounding box
// ============================================================

static openvdb::BBoxd GetWorldBoundingBox(
    const openvdb::FloatGrid::Ptr& grid)
{
    if (!grid)
    {
        throw std::runtime_error(
            "Invalid grid."
        );
    }

    const openvdb::CoordBBox indexBBox =
        grid->evalActiveVoxelBoundingBox();

    if (indexBBox.empty())
    {
        throw std::runtime_error(
            "Grid contains no active voxels."
        );
    }

    const openvdb::Coord& minCoord =
        indexBBox.min();

    const openvdb::Coord& maxCoord =
        indexBBox.max();

    const openvdb::Vec3d minIndex(
        static_cast<double>(minCoord.x()),
        static_cast<double>(minCoord.y()),
        static_cast<double>(minCoord.z())
    );

    const openvdb::Vec3d maxIndex(
        static_cast<double>(maxCoord.x()),
        static_cast<double>(maxCoord.y()),
        static_cast<double>(maxCoord.z())
    );

    const openvdb::Vec3d minWorld =
        grid->indexToWorld(minIndex);

    const openvdb::Vec3d maxWorld =
        grid->indexToWorld(maxIndex);

    return openvdb::BBoxd(
        minWorld,
        maxWorld
    );
}


// ============================================================
// Expand destination bounding box
// ============================================================

static void ExpandBoundingBox(
    openvdb::BBoxd& destination,
    const openvdb::BBoxd& source)
{
    destination.expand(source.min());
    destination.expand(source.max());
}


// ============================================================
// Print grid information
// ============================================================

static void PrintGridInfo(
    openvdb::io::File& file)
{
    std::cout
        << "\nAvailable grids:\n\n";

    for (
        auto iter = file.beginName();
        iter != file.endName();
        ++iter)
    {
        const std::string name =
            iter.gridName();

        openvdb::GridBase::Ptr grid =
            file.readGrid(name);

        if (grid)
        {
            std::cout
                << "Name: "
                << name
                << "\n";

            std::cout
                << "Type: "
                << grid->type()
                << "\n\n";
        }
    }
}


// ============================================================
// Main
// ============================================================

int main(
    int argc,
    char** argv)
{
    try
    {
        // ====================================================
        // Command line
        // ====================================================

        if (argc < 3)
        {
            std::cout
                << "\nUsage:\n\n"
                << "VDBToDDS.exe input.vdb output.dds\n\n";

            return 1;
        }


        const std::string inputFile =
            argv[1];

        const std::wstring outputFile =
            ToWideString(argv[2]);


        // ====================================================
        // Initialize OpenVDB
        // ====================================================

        openvdb::initialize();


        // ====================================================
        // Open VDB file
        // ====================================================

        std::cout
            << "\nOpening VDB file:\n"
            << inputFile
            << "\n";

        openvdb::io::File file(
            inputFile
        );

        file.open();


        // ====================================================
        // Print grid information
        // ====================================================

        PrintGridInfo(file);


        // ====================================================
        // Grid mapping
        //
        // R -> density_scale
        // G -> detail_type
        // B -> dimensional_profile
        // ====================================================

        const std::string gridNameR =
            "density_scale";

        const std::string gridNameG =
            "detail_type";

        const std::string gridNameB =
            "dimensional_profile";


        std::cout
            << "Loading grids:\n"
            << "  R: " << gridNameR << "\n"
            << "  G: " << gridNameG << "\n"
            << "  B: " << gridNameB << "\n";


        // ====================================================
        // Load grids
        // ====================================================

        openvdb::FloatGrid::Ptr gridR =
            openvdb::gridPtrCast<
                openvdb::FloatGrid
            >(
                file.readGrid(gridNameR)
            );


        openvdb::FloatGrid::Ptr gridG =
            openvdb::gridPtrCast<
                openvdb::FloatGrid
            >(
                file.readGrid(gridNameG)
            );


        openvdb::FloatGrid::Ptr gridB =
            openvdb::gridPtrCast<
                openvdb::FloatGrid
            >(
                file.readGrid(gridNameB)
            );


        file.close();


        // ====================================================
        // Validate grids
        // ====================================================

        if (!gridR)
        {
            throw std::runtime_error(
                "Could not load grid: " +
                gridNameR
            );
        }

        if (!gridG)
        {
            throw std::runtime_error(
                "Could not load grid: " +
                gridNameG
            );
        }

        if (!gridB)
        {
            throw std::runtime_error(
                "Could not load grid: " +
                gridNameB
            );
        }


        // ====================================================
        // Calculate shared world-space bounds
        // ====================================================

        std::cout
            << "\nCalculating shared bounds...\n";


        openvdb::BBoxd worldBounds =
            GetWorldBoundingBox(gridR);


        ExpandBoundingBox(
            worldBounds,
            GetWorldBoundingBox(gridG)
        );


        ExpandBoundingBox(
            worldBounds,
            GetWorldBoundingBox(gridB)
        );


        const openvdb::Vec3d worldMin =
            worldBounds.min();

        const openvdb::Vec3d worldMax =
            worldBounds.max();

        const openvdb::Vec3d worldSize =
            worldMax - worldMin;


        std::cout
            << "\nWorld Bounds:\n";

        std::cout
            << "Min: "
            << worldMin
            << "\n";

        std::cout
            << "Max: "
            << worldMax
            << "\n";


        // ====================================================
        // Destination voxel size
        //
        // Texture X -> World X
        // Texture Y -> World Z
        // Texture Z -> World Y
        // ====================================================

        const double voxelSizeX =
            worldSize.x() /
            static_cast<double>(OUTPUT_WIDTH);

        const double voxelSizeY =
            worldSize.y() /
            static_cast<double>(OUTPUT_DEPTH);

        const double voxelSizeZ =
            worldSize.z() /
            static_cast<double>(OUTPUT_HEIGHT);


        std::cout
            << "\nDestination Voxel Size:\n"
            << "X: " << voxelSizeX << "\n"
            << "Y: " << voxelSizeY << "\n"
            << "Z: " << voxelSizeZ << "\n";


        // ====================================================
        // Create OpenVDB samplers
        //
        // BoxSampler provides trilinear interpolation.
        // ====================================================

        using Sampler =
            openvdb::tools::GridSampler<
                openvdb::FloatGrid::TreeType,
                openvdb::tools::BoxSampler
            >;


        Sampler samplerR(
            gridR->tree(),
            gridR->transform()
        );

        Sampler samplerG(
            gridG->tree(),
            gridG->transform()
        );

        Sampler samplerB(
            gridB->tree(),
            gridB->transform()
        );


        // ====================================================
        // Allocate output volume
        // ====================================================

        const size_t voxelCount =
            static_cast<size_t>(OUTPUT_WIDTH)
            *
            static_cast<size_t>(OUTPUT_HEIGHT)
            *
            static_cast<size_t>(OUTPUT_DEPTH);


        std::cout
            << "\nAllocating volume...\n";


        std::vector<VoxelRGBA> volume(
            voxelCount
        );


        // ====================================================
        // Generate filtered volume
        //
        // Each destination voxel is sampled:
        //
        // 3 x 3 x 3 = 27 times
        //
        // Weighted filter:
        //
        // 1, 2, 1
        // ====================================================

        std::cout
            << "\nGenerating "
            << OUTPUT_WIDTH
            << " x "
            << OUTPUT_HEIGHT
            << " x "
            << OUTPUT_DEPTH
            << " weighted filtered RGB volume...\n";


        std::cout
            << "Downsampling: 3 x 3 x 3 = 27 samples per voxel\n";

        std::cout
            << "Filter weights: [1, 2, 1]\n\n";


        // ====================================================
        // Texture Z -> World Y
        // ====================================================

        for (
            uint32_t z = 0;
            z < OUTPUT_DEPTH;
            ++z)
        {
            std::cout
                << "\rProcessing slice "
                << (z + 1)
                << " / "
                << OUTPUT_DEPTH
                << std::flush;


            const double voxelMinY =
                worldMin.y()
                +
                static_cast<double>(z)
                *
                voxelSizeY;


            const double voxelMaxY =
                voxelMinY
                +
                voxelSizeY;


            // =================================================
            // Texture Y -> World Z
            // =================================================

            for (
                uint32_t y = 0;
                y < OUTPUT_HEIGHT;
                ++y)
            {
                const double voxelMinZ =
                    worldMin.z()
                    +
                    static_cast<double>(y)
                    *
                    voxelSizeZ;


                const double voxelMaxZ =
                    voxelMinZ
                    +
                    voxelSizeZ;


                // =============================================
                // Texture X -> World X
                // =============================================

                for (
                    uint32_t x = 0;
                    x < OUTPUT_WIDTH;
                    ++x)
                {
                    const double voxelMinX =
                        worldMin.x()
                        +
                        static_cast<double>(x)
                        *
                        voxelSizeX;


                    const double voxelMaxX =
                        voxelMinX
                        +
                        voxelSizeX;


                    // =========================================
                    // Weighted accumulation
                    // =========================================

                    float sumR = 0.0f;
                    float sumG = 0.0f;
                    float sumB = 0.0f;

                    float totalWeight = 0.0f;


                    // =========================================
                    // 3 x 3 x 3 weighted samples
                    // =========================================

                    for (
                        uint32_t sz = 0;
                        sz < SAMPLE_GRID;
                        ++sz)
                    {
                        for (
                            uint32_t sy = 0;
                            sy < SAMPLE_GRID;
                            ++sy)
                        {
                            for (
                                uint32_t sx = 0;
                                sx < SAMPLE_GRID;
                                ++sx)
                            {
                                // =================================
                                // Normalized positions:
                                //
                                // SAMPLE_GRID = 3
                                //
                                // 1/6
                                // 3/6 = 0.5
                                // 5/6
                                // =================================

                                const double fx =
                                    (
                                        static_cast<double>(sx)
                                        +
                                        0.5
                                    )
                                    /
                                    static_cast<double>(
                                        SAMPLE_GRID
                                    );


                                const double fy =
                                    (
                                        static_cast<double>(sy)
                                        +
                                        0.5
                                    )
                                    /
                                    static_cast<double>(
                                        SAMPLE_GRID
                                    );


                                const double fz =
                                    (
                                        static_cast<double>(sz)
                                        +
                                        0.5
                                    )
                                    /
                                    static_cast<double>(
                                        SAMPLE_GRID
                                    );


                                // =================================
                                // Calculate sample position
                                // =================================

                                const openvdb::Vec3d samplePosition(
                                    voxelMinX
                                    +
                                    (
                                        voxelMaxX
                                        -
                                        voxelMinX
                                    )
                                    *
                                    fx,

                                    voxelMinY
                                    +
                                    (
                                        voxelMaxY
                                        -
                                        voxelMinY
                                    )
                                    *
                                    fy,

                                    voxelMinZ
                                    +
                                    (
                                        voxelMaxZ
                                        -
                                        voxelMinZ
                                    )
                                    *
                                    fz
                                );


                                // =================================
                                // Separable 3D filter weight
                                //
                                // weight =
                                //     weightX *
                                //     weightY *
                                //     weightZ
                                // =================================

                                const float weight =
                                    FILTER_WEIGHTS[sx]
                                    *
                                    FILTER_WEIGHTS[sy]
                                    *
                                    FILTER_WEIGHTS[sz];


                                // =================================
                                // Sample VDB grids
                                // =================================

                                const float valueR =
                                    samplerR.wsSample(
                                        samplePosition
                                    );


                                const float valueG =
                                    samplerG.wsSample(
                                        samplePosition
                                    );


                                const float valueB =
                                    samplerB.wsSample(
                                        samplePosition
                                    );


                                // =================================
                                // Weighted accumulation
                                // =================================

                                sumR +=
                                    valueR
                                    *
                                    weight;


                                sumG +=
                                    valueG
                                    *
                                    weight;


                                sumB +=
                                    valueB
                                    *
                                    weight;


                                totalWeight +=
                                    weight;
                            }
                        }
                    }


                    // =========================================
                    // Normalize weighted result
                    // =========================================

                    const float densityR =
                        sumR
                        /
                        totalWeight;


                    const float densityG =
                        sumG
                        /
                        totalWeight;


                    const float densityB =
                        sumB
                        /
                        totalWeight;


                    // =========================================
                    // BC6H_UF16 only supports unsigned values
                    // =========================================

                    const float finalR =
                        std::max(
                            0.0f,
                            densityR
                        );


                    const float finalG =
                        std::max(
                            0.0f,
                            densityG
                        );


                    const float finalB =
                        std::max(
                            0.0f,
                            densityB
                        );


                    // =========================================
                    // Linear volume index
                    //
                    // X = fastest changing
                    // Y = next
                    // Z = depth slice
                    // =========================================

                    const size_t index =
                        static_cast<size_t>(x)
                        +
                        static_cast<size_t>(y)
                        *
                        OUTPUT_WIDTH
                        +
                        static_cast<size_t>(z)
                        *
                        OUTPUT_WIDTH
                        *
                        OUTPUT_HEIGHT;


                    // =========================================
                    // Store FP16 values
                    // =========================================

                    volume[index].r =
                        FloatToHalf(finalR);

                    volume[index].g =
                        FloatToHalf(finalG);

                    volume[index].b =
                        FloatToHalf(finalB);

                    volume[index].a =
                        FloatToHalf(1.0f);
                }
            }
        }


        std::cout
            << "\n\nVolume generation complete.\n";


        // ====================================================
        // Create DirectXTex 3D texture
        //
        // One mip level only.
        // ====================================================

        std::cout
            << "Creating 3D texture...\n";


        DirectX::ScratchImage sourceImage;


        HRESULT hr =
            sourceImage.Initialize3D(
                DXGI_FORMAT_R16G16B16A16_FLOAT,
                OUTPUT_WIDTH,
                OUTPUT_HEIGHT,
                OUTPUT_DEPTH,
                1
            );


        if (FAILED(hr))
        {
            throw std::runtime_error(
                "Failed to initialize 3D ScratchImage."
            );
        }


        // ====================================================
        // Copy volume data
        // ====================================================

        uint8_t* destination =
            sourceImage.GetPixels();


        if (!destination)
        {
            throw std::runtime_error(
                "Failed to access ScratchImage pixels."
            );
        }


        const size_t totalSize =
            voxelCount
            *
            sizeof(VoxelRGBA);


        std::memcpy(
            destination,
            volume.data(),
            totalSize
        );


        // ====================================================
        // Compress to BC6H
        // ====================================================

        std::cout
            << "Compressing to BC6H_UF16...\n";


        DirectX::ScratchImage compressedImage;


        hr =
            DirectX::Compress(
                sourceImage.GetImages(),
                sourceImage.GetImageCount(),
                sourceImage.GetMetadata(),

                DXGI_FORMAT_BC6H_UF16,

                DirectX::TEX_COMPRESS_PARALLEL,

                DirectX::TEX_THRESHOLD_DEFAULT,

                compressedImage
            );


        if (FAILED(hr))
        {
            throw std::runtime_error(
                "BC6H compression failed."
            );
        }


        // ====================================================
        // Save DDS
        // ====================================================

        std::cout
            << "Writing DDS file...\n";


        hr =
            DirectX::SaveToDDSFile(
                compressedImage.GetImages(),
                compressedImage.GetImageCount(),
                compressedImage.GetMetadata(),

                DirectX::DDS_FLAGS_NONE,

                outputFile.c_str()
            );


        if (FAILED(hr))
        {
            throw std::runtime_error(
                "Failed to save DDS file."
            );
        }


        // ====================================================
        // Success
        // ====================================================

        std::cout
            << "\n========================================\n"
            << "SUCCESS\n"
            << "========================================\n";

        std::cout
            << "Output: "
            << argv[2]
            << "\n";

        std::cout
            << "Resolution: "
            << OUTPUT_WIDTH
            << " x "
            << OUTPUT_HEIGHT
            << " x "
            << OUTPUT_DEPTH
            << "\n";

        std::cout
            << "Format: BC6H_UF16\n";

        std::cout
            << "Mip Levels: 1\n";

        std::cout
            << "Downsampling: 3x3x3 weighted filter\n";

        std::cout
            << "Samples per voxel: 27\n";

        std::cout
            << "Filter weights: [1, 2, 1]\n\n";


        return 0;
    }

    catch (
        const std::exception& exception)
    {
        std::cerr
            << "\n========================================\n"
            << "ERROR\n"
            << "========================================\n";

        std::cerr
            << exception.what()
            << "\n\n";

        return 1;
    }
}