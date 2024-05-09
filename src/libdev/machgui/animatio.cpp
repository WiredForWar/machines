/*
 * A N I M A T I O . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/animatio.hpp"

#include "gui/gui.hpp"
#include "gui/painter.hpp"
#include "device/time.hpp"
#include "system/pathname.hpp"
#include "machgui/internal/mgsndman.hpp"
#include <fstream>

void MachGuiAnimations::init(GuiDisplayable* pParent, const SysPathName& pathName, float scale)
{
    ASSERT_FILE_EXISTS(pathName.c_str());
    std::ifstream in(pathName.c_str());

    int numAnimations;
    in >> numAnimations;
    string animPath;

    animations_.reserve(numAnimations);
    bool colorKey{};
    int xOffset{};
    int yOffset{};

    while (numAnimations--)
    {
        std::istream& tmpIn = in;
        tmpIn >> animPath;
        animations_.push_back(MachGuiAnimation::createAnimation(pParent, animPath, colorKey, xOffset, yOffset, scale));
    }
}

void MachGuiAnimations::update()
{
    // Update all animations
    for (ctl_pvector<MachGuiAnimation>::iterator iter = animations_.begin(); iter != animations_.end(); ++iter)
    {
        (*iter)->update();
    }
}

// static
MachGuiAnimation* MachGuiAnimation::createAnimation(
    GuiDisplayable* pParent,
    const SysPathName& animPath,
    bool colourKey,
    int xOffset,
    int yOffset,
    float scale)
{
    ASSERT_FILE_EXISTS(animPath.c_str());
    std::ifstream in(animPath.c_str());
    std::istream& tmpIn = in;

    int x, y, numCells, loopBack;
    string path;
    double duration, minT, maxT;
    MachGuiAnimation::Cell::CellTime cellTimeType;
    bool hasSound;
    string wavFile;

    in >> x >> y >> numCells >> loopBack;

    // Check that loopBack is set correctly. LoopBack is zeroBased ( i.e. animation with 3 frames can have loopBack
    // set to 0, 1, or 2 NOT 3 ).
    ASSERT(loopBack < numCells, "Loop back set incorrectly, must be less than numCells");

    std::unique_ptr<Cells> pCells = std::make_unique<Cells>();
    pCells->reserve(numCells);

    while (numCells--)
    {
        tmpIn >> path >> duration >> minT >> maxT >> (int&)cellTimeType >> hasSound;
        if (hasSound)
            tmpIn >> wavFile;

        MachGuiAnimation::Cell cell {
            .cell_ = Gui::requestScaledImage(path, scale),
            .duration_ = duration,
            .randomDurationMinTime_ = minT,
            .randomDurationMaxTime_ = maxT,
            .cellTimeType_ = cellTimeType,
            .hasSound_ = hasSound,
            .wavFile_ = wavFile,
        };
        if (colourKey)
        {
            cell.cell_.enableColourKeying();
        }

        pCells->emplace_back(cell);
    }

    Gui::Coord coord(x, y);

    MachGuiAnimation* pAnim = new MachGuiAnimation(
        pParent,
        Gui::Box(Gui::Coord(xOffset, yOffset) + coord * scale, Gui::Size(0, 0)),
        std::move(pCells));
    pAnim->loopBack_ = loopBack;

    return pAnim;
}

MachGuiAnimation::MachGuiAnimation(GuiDisplayable* pParent, const Gui::Box& box, std::unique_ptr<Cells> pCells)
    : GuiDisplayable(pParent, box)
    , pCells_(std::move(pCells))
    , cellIndex_(0)
    , loopBack_(0)
{
    updateCellTimes();

    startTime_ = DevTime::instance().time();

    useFastSecondDisplay(false);

    TEST_INVARIANT;
}

void MachGuiAnimation::update()
{
    size_t oldCellIndex = cellIndex_;

    double newTime = DevTime::instance().time();
    double timeDiff = newTime - startTime_;

    while (timeDiff > pCells_->at(cellIndex_).finishTime_)
    {
        ++cellIndex_;
        // Wrap around to beginning of animation
        if (cellIndex_ >= pCells_->size())
        {
            double overShootTime = timeDiff - pCells_->at(cellIndex_ - 1).finishTime_;
            startTime_ = newTime - overShootTime;
            timeDiff = overShootTime;
            cellIndex_ = loopBack_;

            updateCellTimes();
        }
    }

    if (oldCellIndex != cellIndex_)
    {
        changed();

        if (pCells_->at(cellIndex_).hasSound_)
            playSound(pCells_->at(cellIndex_).wavFile_);
    }
}

void MachGuiAnimation::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiAnimation& t)
{

    o << "MachGuiAnimation " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiAnimation " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiAnimation::doDisplay()
{
    const GuiBitmap& cell = pCells_->at(cellIndex_).cell_;
    if (cell.requestedSize().isNull())
    {
        GuiPainter::instance().blit(cell, absoluteBoundary().minCorner());
    }
    else
    {
        Gui::Size cellSize(cell.requestedSize().width, cell.requestedSize().height);
        GuiPainter::instance().stretch(cell, Gui::Box(absoluteBoundary().minCorner(), cellSize));
    }
}

void MachGuiAnimation::updateCellTimes()
{
    // First of all assign a cell time to any cells with random duration
    for (MachGuiAnimation::Cells::iterator iter = pCells_->begin(); iter != pCells_->end(); ++iter)
    {
        if (iter->cellTimeType_ == Cell::RANDOM)
        {
            double randNum = rand();
            double randNum2 = randNum / RAND_MAX; // Get number from 0.0 - 1.0
            // Get random number to fall between our stated upper and lower bounds
            double diff = iter->randomDurationMaxTime_ - iter->randomDurationMinTime_;
            diff = diff * randNum2;
            diff += iter->randomDurationMinTime_;
            iter->duration_ = diff;
        }
    }

    // Work out finish times
    double finishTime = 0.0;
    int index = 0;
    for (MachGuiAnimation::Cells::iterator iter = pCells_->begin(); iter != pCells_->end(); ++iter)
    {
        ++index;
        if (index > cellIndex_)
        {
            finishTime += iter->duration_;
            iter->finishTime_ = finishTime;
        }
    }
}

void MachGuiAnimation::playSound(const std::string& wavFile)
{
    MachGuiSoundManager::instance().playSound(wavFile);
}

void MachGuiAnimation::jumpToCell(size_t cellIndex)
{
    PRE(pCells_);
    PRE(cellIndex < pCells_->size());

    cellIndex_ = cellIndex;
    startTime_ = DevTime::instance().time();
    updateCellTimes();
}

size_t MachGuiAnimation::cellIndex() const
{
    return cellIndex_;
}

size_t MachGuiAnimation::numCells() const
{
    PRE(pCells_);

    return pCells_->size();
}

/* End ANIMATIO.CPP *************************************************/
