#include <gtest/gtest.h>

#include "render/DisplayModeCatalogue.hpp"

namespace
{

using DisplayMode = Ren::IWindowAdapter::DisplayMode;

DisplayMode mode(int width, int height, int refreshRate, uint32_t format = 0)
{
    return DisplayMode{
        .width = width,
        .height = height,
        .depth = 32,
        .refreshRate = refreshRate,
        .format = format,
    };
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DisplayModeCatalogueTests, AnEmptyModeListMakesAnEmptyCatalogue)
{
    const Ren::DisplayModeCatalogue catalogue{{}};

    EXPECT_TRUE(catalogue.isEmpty());
    EXPECT_TRUE(catalogue.resolutions().empty());
}

TEST(DisplayModeCatalogueTests, RepeatsOfTheSameModeCollapseToOneEntry)
{
    // A display offers the same resolution and rate once per pixel format it can
    // present them in. The player is choosing a resolution, not a pixel format.
    const Ren::DisplayModeCatalogue catalogue{{
        mode(1920, 1080, 60, 1),
        mode(1920, 1080, 60, 2),
        mode(1920, 1080, 60, 3),
    }};

    ASSERT_EQ(1u, catalogue.resolutions().size());
    EXPECT_EQ(Ren::Size(1920, 1080), catalogue.resolutions().front());
    EXPECT_EQ(std::vector<int>({60}), catalogue.refreshRates({1920, 1080}));
}

TEST(DisplayModeCatalogueTests, ResolutionsRunLargestFirst)
{
    const Ren::DisplayModeCatalogue catalogue{{
        mode(640, 480, 60),
        mode(1920, 1080, 60),
        mode(1280, 720, 60),
    }};

    const std::vector<Ren::Size> expected{{1920, 1080}, {1280, 720}, {640, 480}};
    EXPECT_EQ(expected, catalogue.resolutions());
}

TEST(DisplayModeCatalogueTests, ResolutionsOfTheSameAreaRunWidestFirst)
{
    const Ren::DisplayModeCatalogue catalogue{{
        mode(1440, 1440, 60),
        mode(1920, 1080, 60),
    }};

    const std::vector<Ren::Size> expected{{1920, 1080}, {1440, 1440}};
    EXPECT_EQ(expected, catalogue.resolutions());
}

TEST(DisplayModeCatalogueTests, RefreshRatesRunHighestFirst)
{
    const Ren::DisplayModeCatalogue catalogue{{
        mode(1920, 1080, 60),
        mode(1920, 1080, 144),
        mode(1920, 1080, 120),
    }};

    EXPECT_EQ(std::vector<int>({144, 120, 60}), catalogue.refreshRates({1920, 1080}));
}

TEST(DisplayModeCatalogueTests, EachResolutionKeepsOnlyItsOwnRefreshRates)
{
    const Ren::DisplayModeCatalogue catalogue{{
        mode(1920, 1080, 60),
        mode(1920, 1080, 144),
        mode(1280, 720, 75),
    }};

    EXPECT_EQ(std::vector<int>({144, 60}), catalogue.refreshRates({1920, 1080}));
    EXPECT_EQ(std::vector<int>({75}), catalogue.refreshRates({1280, 720}));
}

TEST(DisplayModeCatalogueTests, TheBestRefreshRateIsTheHighestOffered)
{
    const Ren::DisplayModeCatalogue catalogue{{
        mode(1920, 1080, 60),
        mode(1920, 1080, 144),
    }};

    EXPECT_EQ(144, catalogue.bestRefreshRate({1920, 1080}));
}

TEST(DisplayModeCatalogueTests, AResolutionThatIsNotOfferedHasNothingToOffer)
{
    const Ren::DisplayModeCatalogue catalogue{{ mode(1920, 1080, 60) }};

    EXPECT_TRUE(catalogue.refreshRates({800, 600}).empty());
    EXPECT_EQ(0, catalogue.bestRefreshRate({800, 600}));
}

TEST(DisplayModeCatalogueTests, ACatalogueOffersTheResolutionsItHolds)
{
    const Ren::DisplayModeCatalogue catalogue{{ mode(1920, 1080, 60) }};

    EXPECT_TRUE(catalogue.offers({1920, 1080}));
    EXPECT_FALSE(catalogue.offers({800, 600}));
}
