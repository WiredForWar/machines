/*
 * T E X D A T A . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "world4d/internal/texdata.hpp"
#include "world4d/matfrmpl.hpp"
#include "system/pathname.hpp"
#include "render/texmgr.hpp"
#include <cstdlib>

PER_DEFINE_PERSISTENT(W4dCycleTextureData);

W4dCycleTextureData::W4dCycleTextureData(
    const std::string& textureName,
    const size_t& startTexture,
    const size_t& endTexture,
    const size_t& nRepetations)
    : textureName_(textureName)
    , startTexture_(startTexture)
    , endTexture_(endTexture)
    , nRepetations_(nRepetations)
{

    TEST_INVARIANT;
}

W4dCycleTextureData::~W4dCycleTextureData()
{
    TEST_INVARIANT;
}

void W4dCycleTextureData::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dCycleTextureData& t)
{

    o << "W4dCycleTextureData " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dCycleTextureData " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

ctl_vector<RenTexture> W4dCycleTextureData::keyTextures() const
{
    PRE(endTexture_ > startTexture_);

    size_t nameLength = textureName_.length();

    std::string nameRoot;
    std::string extension;

    if (textureName_.substr(nameLength - 2, 2) == "_b")
    {
        nameRoot = textureName_.substr(0, nameLength - 2);
        extension = "_b.bmp";
    }
    else if (textureName_.substr(nameLength - 3, 3) == "_bt")
    {
        nameRoot = textureName_.substr(0, nameLength - 3);
        extension = "_bt.bmp";
    }
    else
    {
        nameRoot = textureName_;
        extension = ".bmp";
    }

    size_t nTextures = endTexture_ - startTexture_ + 1;

    ctl_vector<RenTexture> textures;
    textures.reserve(nTextures);

    for (int i = startTexture_; i < endTexture_ + 1; ++i)
    {
        std::string textureName = nameRoot;

        if (i < 10)
        {
            textureName += "0";
        }

        char textN[4];
        //      itoa(i, textN, 10 );
        sprintf(textN, "%d", i);
        textureName += std::string(textN);

        textureName += extension;

        textures.push_back(RenTexManager::instance().createTexture(textureName));
    }

    return textures;
}

W4dCycleTextureData::W4dCycleTextureData(PerConstructor)
{
}

void perWrite(PerOstream& str, const W4dCycleTextureData& t)
{
    writeAllocatedStringFromPointer(str, &t.textureName_);
    str << t.startTexture_;
    str << t.endTexture_;
    str << t.nRepetations_;
}

void perRead(PerIstream& str, W4dCycleTextureData& t)
{
    // t.textureName_ used to be a heap-allocated string. Use special read function
    // to load PerDataType::PER_OBJECT_POINTER into a stack-allocated object
    readAllocatedStringFromPointer(str, &t.textureName_);
    str >> t.startTexture_;
    str >> t.endTexture_;
    str >> t.nRepetations_;
}

/* End TEXDATA.CPP **************************************************/
