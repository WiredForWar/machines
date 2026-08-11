/*
 * D I S P L A Y A B L E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include "gui/Displayable.hpp"
#include "ctl/Vector.hpp"
#include "ctl/Algorithm.hpp"
#include "mathex/Vec2.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "render/Device.hpp"
#include "gui/Manager.hpp"
#include "gui/GuiPainter.hpp"
#include "device/ButtonEvent.hpp"

#include <ranges>

//////////////////////////////////////////////////////////////////////

GuiDisplayable::GuiDisplayable(GuiDisplayable* pParent, Layer layer)
    : pParent_(pParent)
{
    for (Layer childLayer : AllLayers)
    {
        children_[childLayer].reserve(2);
    }
    allChildren_.reserve(4);

    changed(true);

    if (pParent)
    {
        pParent_->addChild(this, layer);
    }
}

GuiDisplayable::GuiDisplayable(GuiDisplayable* pParent, const Gui::Boundary& relativeBoundary, Layer myLayer)
    : GuiDisplayable(pParent, myLayer)
{
    PRE(pParent != nullptr);

    setRelativeBoundary(relativeBoundary);
    POST_INFO(pParent->absoluteBoundary());
    POST_INFO(absoluteBoundary());
    POST(pParent->absoluteBoundary().contains(absoluteBoundary()));
    POST(useFastSecondDisplay());
}

GuiDisplayable::GuiDisplayable(const Gui::Boundary& absBoundary)
    : GuiDisplayable(nullptr)
{
    relativeBox_ = absBoundary;
    absoluteBox_ = absBoundary;

    POST(useFastSecondDisplay());
}

GuiDisplayable::~GuiDisplayable()
{
    deleteAllChildren();

    if (pParent_ != nullptr)
        pParent_->removeChild(this);

    if (acceptsFocus_)
        GuiManager::instance().removeFromFocusChain(this);

    GuiManager::instance().isBeingDeleted(this);
}

//////////////////////////////////////////////////////////////////////

Gui::Coord GuiDisplayable::relativeCoord() const
{
    return relativeBox_.minCorner();
}

Gui::Coord GuiDisplayable::absoluteCoord() const
{
    return absoluteBox_.minCorner();
}

void GuiDisplayable::relativeCoord(const Gui::Coord& relCoord)
{
    // Construct the displacement vector
    MexVec2 diff(relativeBox_.minCorner(), relCoord);
    PRE_INFO(relativeBox_.minCorner());
    PRE_INFO(relCoord);

    POST_DATA(Gui::Box oldBoundary = absoluteBoundary());

    PRE_DATA(Gui::Coord c(diff.x(), diff.y()));
    PRE_DATA(Gui::Box parentBoundary = isRoot() ? absoluteBoundary() : parent()->absoluteBoundary());
    PRE_DATA(Gui::Box newBoundary = translateBox(absoluteBoundary(), c));
    PRE_INFO(c);
    PRE_INFO(absoluteBoundary());
    PRE_INFO(parentBoundary);
    PRE_INFO(newBoundary);

    Gui::Coord newRelMin = relativeBox_.minCorner();
    newRelMin += diff;
    Gui::Coord newRelMax = relativeBox_.maxCorner();
    newRelMax += diff;
    relativeBox_.corners(newRelMin, newRelMax);

    Gui::Coord absCoord = relativeCoord();
    if (! isRoot())
        absCoord += parent()->absoluteCoord();

    absoluteBox_ = Gui::Box(absCoord, relativeBox_.size());

    for (Children::iterator i = allChildren_.begin(); i != allChildren_.end(); ++i)
    {
        Gui::Coord childCoord = (*i)->absoluteCoord();
        childCoord += diff;
        positionChildAbsolute(*i, childCoord);
    }
}

void GuiDisplayable::absoluteCoord(const Gui::Coord& absCoord)
{
    // Construct the displacement vector
    MexVec2 diff(absoluteBox_.minCorner(), absCoord);

    Gui::Coord newAbsMin = absoluteBox_.minCorner();
    newAbsMin += diff;

    Gui::Coord newAbsMax = absoluteBox_.maxCorner();
    newAbsMax += diff;
    absoluteBox_.corners(newAbsMin, newAbsMax);

    Gui::Coord relCoord = absCoord;
    if (! isRoot())
        relCoord -= parent()->absoluteCoord();
    relativeBox_ = Gui::Box(relCoord, absoluteBox_.size());

    for (Children::iterator i = allChildren_.begin(); i != allChildren_.end(); ++i)
    {
        Gui::Coord childCoord = (*i)->absoluteCoord();
        childCoord += diff;
        positionChildAbsolute(*i, childCoord);
    }

    POST(absoluteCoord() == absCoord);
    POST(absoluteBoundary() == Gui::Box(absCoord, width(), height()));
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::positionChildAbsolute(GuiDisplayable* pChild, const Gui::Coord& absCoord)
{
    PRE(pChild != nullptr);
    PRE(hasChild(pChild));

    pChild->absoluteCoord(absCoord);

    POST(pChild->absoluteCoord() == absCoord);
}

void GuiDisplayable::positionChildRelative(GuiDisplayable* pChild, const Gui::Coord& relCoord)
{
    PRE(pChild != nullptr);
    PRE(hasChild(pChild));

    pChild->relativeCoord(relCoord);

    POST(pChild->relativeCoord() == relCoord);
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::setVisible(bool visible)
{
    bool makeVisible = visible && isEligableForVisibility() && ! isVisible_;

    if (makeVisible)
        changed(true);

    isVisible_ = visible;
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::isVisible() const
{
    bool parentVisible = true;

    // Check parents visibility
    if (pParent_)
        parentVisible = pParent_->isVisible();

    // Return true if "this" is visible and my parent(s) is visible etc.
    return isVisible_ && parentVisible && isEligableForVisibility();
}

// virtual
bool GuiDisplayable::isEligableForVisibility() const
{
    return true;
}

void GuiDisplayable::changed(bool change)
{
    if (change)
    {
        // This indicates that the gui displayable needs to be rendered twice ( back buffer and front buffer )
        if (useFourTimesRender())
        {
            changed_ = 4;
        }
        else
        {
            changed_ = 2;
        }
    }
    else if (changed_ != 0)
    {
        --changed_;
    }
}

bool GuiDisplayable::hasChanged() const
{
    return changed_ != 0;
}

bool GuiDisplayable::isRoot() const
{
    return pParent_ == nullptr;
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::addChild(GuiDisplayable* pNewChild, Layer childsLayer)
{
    PRE(! hasChild(pNewChild));
    PRE_INFO(absoluteBoundary());
    PRE_INFO(pNewChild->absoluteBoundary());

    allChildren_.push_back(pNewChild);
    children_[childsLayer].push_back(pNewChild);

    POST(hasChild(pNewChild));
}

// virtual
void GuiDisplayable::doRemoveChild(GuiDisplayable* /*pChild*/)
{
}

void GuiDisplayable::removeChild(GuiDisplayable* pChild)
{
    PRE(hasChild(pChild));

    Children::iterator i = find(allChildren_.begin(), allChildren_.end(), pChild);

    if (i != allChildren_.end())
    {
        doRemoveChild(pChild);
        allChildren_.erase(i);
    }

    for (Layer layer : AllLayers)
    {
        Children::iterator i = find(children_[layer].begin(), children_[layer].end(), pChild);

        if (i != children_[layer].end())
        {
            children_[layer].erase(i);
            break;
        }
    }

    POST(! hasChild(pChild));
}

void GuiDisplayable::reparentChild(GuiDisplayable* pChild, Layer layer)
{
    PRE(pChild != nullptr);

    GuiDisplayable* pOldParent = pChild->parent();
    if (pOldParent == this)
        return;

    if (pOldParent != nullptr)
    {
        pOldParent->removeChild(pChild);
        pChild->pParent_ = nullptr;
    }

    addChild(pChild, layer);
    pChild->pParent_ = this;

    POST(hasChild(pChild));
}

void GuiDisplayable::detachFromParent()
{
    if (pParent_ != nullptr)
    {
        pParent_->removeChild(this);
        pParent_ = nullptr;
    }
}

void GuiDisplayable::deleteChild(GuiDisplayable* pChild)
{
    Children::iterator i = find(allChildren_.begin(), allChildren_.end(), pChild);

    if (i != allChildren_.end())
    {
        delete *i; // Child automatically removes itself from allChildren_ collection on deletion
    }

    POST(! hasChild(pChild));
}

void GuiDisplayable::deleteAllChildren()
{
    // This works because on deletion the child will remove itself from the parents child collection.
    while (allChildren_.size() != 0)
        delete *allChildren_.begin();
}

//////////////////////////////////////////////////////////////////////

GuiDisplayable::Children& GuiDisplayable::children()
{
    return allChildren_;
}

const GuiDisplayable::Children& GuiDisplayable::children() const
{
    return allChildren_;
}

GuiDisplayable::Children& GuiDisplayable::children(Layer layer)
{
    return children_[layer];
}

const GuiDisplayable::Children& GuiDisplayable::children(Layer layer) const
{
    return children_[layer];
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::hasChild(const GuiDisplayable* pChild) const
{
    bool result = false;

    Children::const_iterator i = find(allChildren_.begin(), allChildren_.end(), pChild);

    if (i != allChildren_.end())
    {
        result = true;
    }

    return result;
}

bool GuiDisplayable::recursivelyHasChild(const GuiDisplayable* pChild) const
{
    bool found = false;

    if (this == pChild)
    {
        found = true;
    }

    for (Children::const_iterator i = allChildren_.begin(); ! found && i != allChildren_.end(); ++i)
    {
        found = recursivelyHasChild(*i);
    }

    return found;
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::display()
{
    if (isVisible()) // No point continuing if this displayable is invisible
    {
        // If this has changed and needs rendering then draw it followed by all it's children
        if (hasChanged() || redrawEveryFrame())
        {
            if (secondDisplay() && useFastSecondDisplay() && ! redrawEveryFrame())
            {
                fastDisplay();
            }
            else
            {
                normalDisplay();
            }

            changed(false);
        }
        else
        {
            // Check children to see if they need displaying
            for (Layer layer : AllLayers)
            {
                for (Children::iterator i = children_[layer].begin(); i != children_[layer].end(); ++i)
                {
                    // Call childs display method.
                    (*i)->display();
                }
            }
        }
    }
}

void GuiDisplayable::normalDisplay()
{
    doDisplay();

    // Display all children
    for (Layer layer : AllLayers)
    {
        for (Children::iterator i = children_[layer].begin(); i != children_[layer].end(); ++i)
        {
            if ((*i)->isVisible())
            {
                // If the parent ( this ) has just changed ( i.e. this is the first display out of two )
                // then tell all children to display for the next 2 frames.
                if (firstDisplay() || redrawEveryFrame())
                {
                    (*i)->changed(true);
                }
                // Call childs display method.
                (*i)->display();
            }
        }
    }
}

void GuiDisplayable::fastDisplay()
{
    // Blit from front to back buffer.
    GuiBitmap frontBuffer = W4dManager::instance().sceneManager()->pDevice()->frontSurface();
    GuiPainter::instance().blit(frontBuffer, absoluteBoundary(), absoluteBoundary().minCorner());

    // Display children if necessary
    fastDisplayChildren();
}

void GuiDisplayable::fastDisplayChildren()
{
    for (Layer layer : AllLayers)
    {
        for (Children::iterator i = children_[layer].begin(); i != children_[layer].end(); ++i)
        {
            GuiDisplayable* pChild = (*i);
            if (pChild->isVisible())
            {
                if (pChild->firstDisplay() || pChild->redrawEveryFrame()
                    || (pChild->secondDisplay() && ! pChild->useFastSecondDisplay()))
                {
                    // Call childs display method.
                    pChild->display();
                }
                else
                {
                    // Don't render child ( taken care of by fast display of parent! )
                    pChild->changed(false);
                    pChild->fastDisplayChildren();
                }
            }
        }
    }
}

bool GuiDisplayable::firstDisplay() const
{
    return changed_ > 1;
}

bool GuiDisplayable::secondDisplay() const
{
    return changed_ == 1;
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::empty() const
{
    return allChildren_.empty();
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::filledRectangle(const Gui::Box& rel, const Gui::Colour& c)
{
    GuiPainter::instance().filledRectangle(translate(rel), c);
}

void GuiDisplayable::hollowRectangle(const Gui::Box& rel, const Gui::Colour& c, unsigned thickness)
{
    GuiPainter::instance().hollowRectangle(translate(rel), c, thickness);
}

void GuiDisplayable::fill(const Gui::Colour& c)
{
    GuiPainter::instance().filledRectangle(absoluteBoundary(), c);
}

//////////////////////////////////////////////////////////////////////

Gui::Coord GuiDisplayable::translate(Gui::XCoord x, Gui::YCoord y) const
{
    return Gui::Coord(absoluteCoord().x() + x, absoluteCoord().y() + y);
}

Gui::Coord GuiDisplayable::translate(const Gui::Coord& r) const
{
    return Gui::Coord(absoluteCoord().x() + r.x(), absoluteCoord().y() + r.y());
}

Gui::Box GuiDisplayable::translate(const Gui::Box& b) const
{
    return GuiDisplayable::translateBox(b, absoluteCoord());
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::line(const Gui::Coord& rel1, const Gui::Coord& rel2, const Gui::Colour& c, unsigned thickness)
{
    //  PRE( absoluteBoundary().contains( translate( rel1 ) ) );
    //  PRE( absoluteBoundary().contains( translate( rel2 ) ) );

    GuiPainter::instance().line(translate(rel1), translate(rel2), c, thickness);
}

void GuiDisplayable::horizontalLine(const Gui::Coord& rel1, unsigned length, const Gui::Colour& c, unsigned thickness)
{
    //  PRE( absoluteBoundary().contains( translate( rel1 ) ) );
    //  PRE( absoluteBoundary().contains( translate( rel1.x() + length, rel1.y() ) ) );

    GuiPainter::instance().horizontalLine(translate(rel1), length, c, thickness);
}

void GuiDisplayable::verticalLine(const Gui::Coord& rel1, unsigned height, const Gui::Colour& c, unsigned thickness)
{
    //  PRE( absoluteBoundary().contains( translate( rel1 ) ) );
    //  PRE( absoluteBoundary().contains( translate( rel1.x(), rel1.y() + height ) ) );

    GuiPainter::instance().verticalLine(translate(rel1), height, c, thickness);
}

void GuiDisplayable::bevel(const Gui::Box& rel, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol)
{
    GuiPainter::instance().bevel(translate(rel), thickness, hiCol, loCol);
}

//////////////////////////////////////////////////////////////////////

GuiDisplayable* GuiDisplayable::parent()
{
    return pParent_;
}

const GuiDisplayable* GuiDisplayable::parent() const
{
    return pParent_;
}

bool GuiDisplayable::isEnabled() const
{
    return enabled_;
}

void GuiDisplayable::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

//////////////////////////////////////////////////////////////////////

const Gui::Boundary& GuiDisplayable::absoluteBoundary() const
{
    return absoluteBox_;
}

void GuiDisplayable::setRelativeBoundary(const Gui::Boundary& boundary)
{
    const Gui::Coord oldAbsCoord = absoluteBox_.minCorner();
    const Gui::Size oldSize = relativeBox_.size();

    relativeBox_ = boundary;
    if (pParent_)
    {
        absoluteBox_ = translateBox(boundary, pParent_->absoluteCoord());
    }
    else
    {
        absoluteBox_ = boundary;
    }

    // Children are placed against this one, so they travel with it.
    const MexVec2 diff(oldAbsCoord, absoluteBox_.minCorner());
    if (diff.x() != 0 || diff.y() != 0)
    {
        for (GuiDisplayable* pChild : allChildren_)
        {
            Gui::Coord childCoord = pChild->absoluteCoord();
            childCoord += diff;
            positionChildAbsolute(pChild, childCoord);
        }
    }

    if (relativeBox_.size() != oldSize)
        doResized();
}

void GuiDisplayable::doResized()
{
}

const Gui::Boundary& GuiDisplayable::relativeBoundary() const
{
    return relativeBox_;
}

Gui::Boundary GuiDisplayable::relativeBoundary(const GuiDisplayable& ancestor) const
{
    Gui::Coord coord = relativeCoord(ancestor);

    Gui::Box box(coord, width(), height());

    return box;
}

Gui::Coord GuiDisplayable::relativeCoord(const GuiDisplayable& ancestor) const
{
    PRE(! isRoot());

    Gui::Coord coord = relativeCoord();

    if (parent() != &ancestor)
    {
        Gui::Coord parentCoord = parent()->relativeCoord(ancestor);
        coord.x(coord.x() + parentCoord.x());
        coord.y(coord.y() + parentCoord.y());
    }

    return coord;
}

unsigned GuiDisplayable::width() const
{
    return relativeBox_.width();
}

unsigned GuiDisplayable::height() const
{
    return relativeBox_.height();
}

Gui::Size GuiDisplayable::size() const
{
    return relativeBox_.size();
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::contains(Gui::XCoord x, Gui::YCoord y) const
{
    return contains(Gui::Coord(x, y));
}

// virtual
bool GuiDisplayable::contains(const Gui::Coord& c) const
{
    if (c.x() >= absoluteBoundary().minCorner().x() + width() || c.y() >= absoluteBoundary().minCorner().y() + height())
        return false;

    return absoluteBoundary().contains(c);
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::doHandleKeyEvent(const GuiKeyEvent& gke)
{
    PRE(gke.buttonEvent().isKeyEvent());

    // Default implementation does not "use" the GuiKeyEvent therefore false is returned.
    return false;
}

bool GuiDisplayable::doHandleCharEvent(const GuiCharEvent& gce)
{
    PRE(gce.isCharEvent());

    // Default implementation does not "use" the GuiCharEvent therefore false is returned.
    return false;
}

void GuiDisplayable::doHandleMouseClickEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleMouseEnterEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleMouseExitEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleMouseScrollEvent(GuiMouseEvent* event)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleContainsMouseEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

//////////////////////////////////////////////////////////////////////

// static
Gui::Box GuiDisplayable::translateBox(const Gui::Box& b, const Gui::Coord& c)
{
    MexPoint2d minC = b.minCorner();
    minC += c;
    MexPoint2d maxC = b.maxCorner();
    maxC += c;
    return Gui::Box(minC, maxC);
}

GuiDisplayable* GuiDisplayable::innermostContaining(const Gui::Coord& c)
{
    // If we are visible and the point is contained in the boundary then we have found
    // a gui displayable that contains the mouse.
    if (!isVisible() || !absoluteBoundary().contains(c))
        return nullptr;

    for (Layer layer : AllLayers | std::views::reverse)
    {
        // Check to see if any of the children contain the mouse pointer
        for (GuiDisplayable* pChild : children_[layer])
        {
            GuiDisplayable* result = pChild->innermostContaining(c);
            if (result)
            {
                return result;
            }
        }
    }

    // No children contain coord therefore we are most derived displayable containing coord.
    return this;
}

void GuiDisplayable::changed()
{
    changed(true);
}

void GuiDisplayable::changedIncludingChildren()
{
    changed(true);

    for (GuiDisplayable* pChild : children())
        pChild->changedIncludingChildren();
}

void GuiDisplayable::setLayer(Layer layer)
{
    PRE(! isRoot());

    parent()->removeChild(this);
    parent()->addChild(this, layer);
}

bool GuiDisplayable::redrawEveryFrame() const
{
    return redrawEveryFrame_;
}

void GuiDisplayable::redrawEveryFrame(bool redraw)
{
    redrawEveryFrame_ = redraw;
}

bool GuiDisplayable::useFastSecondDisplay() const
{
    return useFastSecondDisplay_;
}

void GuiDisplayable::useFastSecondDisplay(bool fast)
{
    useFastSecondDisplay_ = fast;
}

#ifndef _PRODUCTION_RELEASE
// virtual
const char* GuiDisplayable::description() const
{
    return "GuiDisplayable";
}
#endif

GuiDisplayable* GuiDisplayable::innermostContainingCheckProcessesMouseEvents(const Gui::Coord& c)
{
    // If we are visible and the point is contained in the boundary then we have found
    // a gui displayable that contains the mouse.
    if (!isVisible() || !absoluteBoundary().contains(c) || !processesMouseEvents())
        return nullptr;

    for (Layer layer : AllLayers | std::views::reverse)
    {
        // Check to see if any of the children contain the mouse pointer
        for (GuiDisplayable* pChild : children_[layer])
        {
            GuiDisplayable* result = pChild->innermostContainingCheckProcessesMouseEvents(c);
            if (result)
            {
                return result;
            }
        }
    }

    // No children contain coord therefore we are most derived displayable containing coord.
    return this;
}

// virtual
bool GuiDisplayable::processesMouseEvents() const
{
    return true;
}

// static
bool& GuiDisplayable::useFourTimesRender()
{
    static bool useFourTimesRenderBool = false;
    return useFourTimesRenderBool;
}

GuiDisplayable* GuiDisplayable::findRoot(GuiDisplayable* current)
{
    if (current->isRoot())
    {
        return current;
    }

    return findRoot(current->parent());
}

//////////////////////////////////////////////////////////////////////

/* End **************************************************************/

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::acceptsFocus() const
{
    return acceptsFocus_;
}

void GuiDisplayable::setAcceptsFocus(bool accepts)
{
    if (acceptsFocus_ == accepts)
        return;

    acceptsFocus_ = accepts;

    if (accepts)
        GuiManager::instance().addToFocusChain(this);
    else
        GuiManager::instance().removeFromFocusChain(this);
}

bool GuiDisplayable::isFocusEnabled() const
{
    return !focusSuppressed_;
}

bool GuiDisplayable::isFocusControl() const
{
    return hasFocus_ && !focusSuppressed_;
}

bool GuiDisplayable::hasFocusSet() const
{
    return hasFocus_;
}

void GuiDisplayable::hasFocus(bool newValue)
{
    hasFocus_ = newValue;
}

bool GuiDisplayable::executeControl()
{
    PRE(isFocusControl());
    PRE(isFocusEnabled());

    return false;
}

bool GuiDisplayable::doHandleNavigationKey(NavKey navKey, GuiDisplayable** ppNavFocus)
{
    switch (navKey)
    {
        case NavKey::LEFT_ARROW:
            *ppNavFocus = pLeftNavControl_;
            break;
        case NavKey::RIGHT_ARROW:
            *ppNavFocus = pRightNavControl_;
            break;
        case NavKey::UP_ARROW:
            *ppNavFocus = pUpNavControl_;
            break;
        case NavKey::DOWN_ARROW:
            *ppNavFocus = pDownNavControl_;
            break;
        case NavKey::TAB_BACKWARD:
            *ppNavFocus = pTabBackwardNavControl_;
            break;
        case NavKey::TAB_FOWARD:
            *ppNavFocus = pTabFowardNavControl_;
            break;
    }

    return (*ppNavFocus != nullptr) && (*ppNavFocus)->isFocusEnabled();
}

void GuiDisplayable::setTabFowardNavControl(GuiDisplayable* pNewValue)
{
    pTabFowardNavControl_ = pNewValue;
}

void GuiDisplayable::setTabBackwardDownNavControl(GuiDisplayable* pNewValue)
{
    pTabBackwardNavControl_ = pNewValue;
}

void GuiDisplayable::setLeftNavControl(GuiDisplayable* pNewValue)
{
    pLeftNavControl_ = pNewValue;
}

void GuiDisplayable::setRightNavControl(GuiDisplayable* pNewValue)
{
    pRightNavControl_ = pNewValue;
}

void GuiDisplayable::setUpNavControl(GuiDisplayable* pNewValue)
{
    pUpNavControl_ = pNewValue;
}

void GuiDisplayable::setDownNavControl(GuiDisplayable* pNewValue)
{
    pDownNavControl_ = pNewValue;
}

void GuiDisplayable::suppressFocus(bool newValue)
{
    focusSuppressed_ = newValue;
}

void GuiDisplayable::escapeControl(bool newValue)
{
    escapeControl_ = newValue;
}

bool GuiDisplayable::isEscapeControl() const
{
    return escapeControl_;
}

void GuiDisplayable::defaultControl(bool newValue)
{
    defaultControl_ = newValue;
}

bool GuiDisplayable::isDefaultControl() const
{
    return defaultControl_;
}
