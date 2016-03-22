//
//  DBBone.cpp
//  dragonbones
//
//  Created by Relvin on 16/2/4.
//  Copyright © 2016年 Relvin. All rights reserved.
//

#include "DBBone.h"
#include "DBArmature.h"
#include "animation/DBAnimationState.h"
NAME_SPACE_DRAGON_BONES_BEGIN

DBBone* DBBone::create(BoneData* boneData)
{
    DBBone* bone = new (std::nothrow) DBBone();
    if (bone && bone->initWithBoneData(boneData))
    {
        bone->autorelease();
        return bone;
    }
    CC_SAFE_DELETE(bone);
    return nullptr;
}

bool DBBone::initWithBoneData(BoneData* boneData)
{
    if (!Node::init())
    {
        return false;
    }
    _name = boneData->name;
    this->inheritRotation = boneData->inheritRotation;
    this->inheritScale = boneData->inheritScale;
    origin = boneData->transform;
    _boneData = boneData;
    
    return true;
}

DBBone::DBBone()
: _boneData (nullptr)
, _pArmature (nullptr)
, displayController("")
,applyOffsetTranslationToChild(true)
,applyOffsetRotationToChild(true)
,applyOffsetScaleToChild(false)
,_isColorChanged(false)
,_needUpdate(2)
,_tween()
{

    inheritScale = false;
    _timelineStateList.clear();
    _childBones.clear();
}


DBBone::~DBBone()
{
    _timelineStateList.clear();
    _childBones.clear();
}


void DBBone::addChildBone(DBBone *child)
{
    CCASSERT( nullptr != child, "Argument must be non-nil");
    CCASSERT( nullptr == child->_parentBone, "child already added. It can't be added again");
    //need rewrite (not use _childern)
    if(_childBones.empty())
    {
        _childBones.reserve(4);
    }
    
    
    
    if (_childBones.getIndex(child) == cocos2d::CC_INVALID_INDEX)
    {
        _childBones.pushBack(child);
        child->setParentBone(this);
    }
}

void DBBone::removeChildBone(DBBone *bone, bool recursion)
{
    if (!_childBones.empty() && _childBones.getIndex(bone) != cocos2d::CC_INVALID_INDEX )
    {
        if(recursion)
        {
            auto ccbones = bone->_childBones;
            
            for(auto& object : ccbones)
            {
                DBBone *ccBone = static_cast<DBBone*>(object);
                bone->removeChildBone(ccBone, recursion);
            }
        }
        
        bone->setParentBone(nullptr);
        
//        bone->getDisplayManager()->setCurrentDecorativeDisplay(nullptr);
        
        _childBones.eraseObject(bone);
    }
}

void DBBone::setArmature(DBArmature *armature)
{
    if (_pArmature == armature)
        return;
    if (_armature)
    {
        
    }
    
    _pArmature = armature;
}

void DBBone::calculateParentTransform( Transform &transform, Matrix &matrix )
{
    DBBone* parentBone = dynamic_cast<DBBone* >(_parentBone);
    if (parentBone && (inheritTranslation || inheritRotation || inheritScale))
    {
        
        transform = parentBone->_globalTransformForChild;
        matrix = parentBone->_globalTransformMatrixForChild;

        if (!inheritTranslation || !inheritRotation || !inheritScale)
        {
            if (!inheritTranslation)
            {
                transform.x = 0.f;
                transform.y = 0.f;
            }
            if (!inheritScale)
            {
                transform.scaleX = 1.f;
                transform.scaleY = 1.f;
            }
            if (!inheritRotation)
            {
                transform.skewX = 0.f;
                transform.skewY = 0.f;
            }
            transform.toMatrix(matrix, true);
        }
    }
}

void DBBone::update(float delta)
{
    
    static int boneCount = 0;
    _needUpdate --;
    
    DBBone* parentBone = dynamic_cast<DBBone*>(_parentBone);
    if (delta || _needUpdate > 0 || (parentBone && parentBone->_needUpdate > 0))
    {
        _needUpdate = 1;
    }
    else
    {
        return;
    }
    
    this->setPosition(global.x,-global.y);
    
#define showBone 1
#ifdef showBone
    cocos2d::Sprite* boneSprite = dynamic_cast<cocos2d::Sprite*>(this->getChildByTag(1001));
    if (!boneSprite)
    {
        boneSprite = cocos2d::Sprite::create("res/bone.png");
        this->addChild(boneSprite);
        boneSprite->setGlobalZOrder(100);
        boneSprite->setTag(1001);
        boneSprite->setAnchorPoint(cocos2d::Vec2(0,0));
        boneSprite->setColor(cocos2d::Color3B(172,172,172));
        boneCount++;
        printf("bone count = %d\n",boneCount);
    }
    if(_boneData->length)
    {
        boneSprite->setScaleX(_boneData->length / 15.0f);
        
    }
    boneSprite->setRotationSkewX(global.skewX * RADIAN_TO_ANGLE);
    boneSprite->setRotationSkewY(global.skewY * RADIAN_TO_ANGLE);
#endif
    
    
    blendingTimeline();
    
    // 计算global
    Transform parentGlobalTransform;
    Matrix parentGlobalTransformMatrix;
    updateGlobal(parentGlobalTransform, parentGlobalTransformMatrix);
    
    // 计算globalForChild
    bool ifExistOffsetTranslation = offset.x != 0 || offset.y != 0;
    bool ifExistOffsetScale = offset.scaleX != 1 || offset.scaleY != 1;
    bool ifExistOffsetRotation = offset.skewX != 0 || offset.skewY != 0;
    
    if(	(!ifExistOffsetTranslation || applyOffsetTranslationToChild) &&
       (!ifExistOffsetScale || applyOffsetScaleToChild) &&
       (!ifExistOffsetRotation || applyOffsetRotationToChild))
    {
        _globalTransformForChild = global;
        _globalTransformMatrixForChild = globalTransformMatrix;
    }
    else
    {
        
        _globalTransformForChild.x = origin.x + _tween.x;
        _globalTransformForChild.y = origin.y + _tween.y;
        _globalTransformForChild.scaleX = origin.scaleX * _tween.scaleX;
        _globalTransformForChild.scaleY = origin.scaleY * _tween.scaleY;
        _globalTransformForChild.skewX = origin.skewX + _tween.skewX;
        _globalTransformForChild.skewY = origin.skewY + _tween.skewY;
        
        if(applyOffsetTranslationToChild)
        {
            _globalTransformForChild.x += offset.x;
            _globalTransformForChild.y += offset.y;
        }
        if(applyOffsetScaleToChild)
        {
            _globalTransformForChild.scaleX *= offset.scaleX;
            _globalTransformForChild.scaleY *= offset.scaleY;
        }
        if(applyOffsetRotationToChild)
        {
            _globalTransformForChild.skewX += offset.skewX;
            _globalTransformForChild.skewY += offset.skewY;
        }
        
        _globalTransformForChild.toMatrix(_globalTransformMatrixForChild, true);
        
        Transform::matrixToTransform(
                                     _globalTransformMatrixForChild, 
                                     _globalTransformForChild,
                                     _globalTransformForChild.scaleX * parentGlobalTransform.scaleX >= 0,
                                     _globalTransformForChild.scaleY * parentGlobalTransform.scaleY >= 0);
    }
    for(const auto &obj: _childBones) {
        DBBone *childBone = static_cast<DBBone*>(obj);
        if (childBone)
        {
            childBone->update(delta);
        }
    }
}

void DBBone::blendingTimeline()
{
    size_t i = _timelineStateList.size();
    if (i == 1)
    {
        DBTimelineState *timelineState = _timelineStateList[0];
        const Transform &transform = timelineState->getTransform();
        const Point &pivot = timelineState->getPivot();
        timelineState->setWeight(timelineState->getAnimationState()->getCurrentWeight());
        const float weight = timelineState->getWeight();
        _tween.x = transform.x * weight;
        _tween.y = transform.y * weight;
        _tween.skewX = transform.skewX * weight;
        _tween.skewY = transform.skewY * weight;
        _tween.scaleX = 1 + (transform.scaleX - 1) * weight;
        _tween.scaleY = 1 + (transform.scaleY - 1) * weight;
        _tweenPivot.x = pivot.x * weight;
        _tweenPivot.y = pivot.y * weight;
    }
    else if (i > 1)
    {
        int prevLayer = _timelineStateList[i - 1]->getAnimationState()->getLayer();
        int currentLayer = 0;
        float weigthLeft = 1.f;
        float layerTotalWeight = 0.f;
        float x = 0.f;
        float y = 0.f;
        float skewX = 0.f;
        float skewY = 0.f;
        float scaleX = 1.f;
        float scaleY = 1.f;
        float pivotX = 0.f;
        float pivotY = 0.f;
        
        while (i--)
        {
            DBTimelineState *timelineState = _timelineStateList[i];
            currentLayer = timelineState->getAnimationState()->getLayer();
            
            if (prevLayer != currentLayer)
            {
                if (layerTotalWeight >= weigthLeft)
                {
                    timelineState->setWeight(0);
                    break;
                }
                else
                {
                    weigthLeft -= layerTotalWeight;
                }
            }
            
            prevLayer = currentLayer;
            timelineState->setWeight(timelineState->getAnimationState()->getCurrentWeight() * weigthLeft);
            const float weight = timelineState->getWeight();
            
            //timelineState
            if (weight && timelineState->getBlendEnabled())
            {
                const Transform &transform = timelineState->getTransform();
                const Point &pivot = timelineState->getPivot();
                x += transform.x * weight;
                y += transform.y * weight;
                skewX += transform.skewX * weight;
                skewY += transform.skewY * weight;
                scaleX += (transform.scaleX - 1) * weight;
                scaleY += (transform.scaleY - 1) * weight;
                pivotX += pivot.x * weight;
                pivotY += pivot.y * weight;
                layerTotalWeight += weight;
            }
        }
        
        _tween.x = x;
        _tween.y = y;
        _tween.skewX = skewX;
        _tween.skewY = skewY;
        _tween.scaleX = scaleX;
        _tween.scaleY = scaleY;
        _tweenPivot.x = pivotX;
        _tweenPivot.y = pivotY;
    }
}

const Point& DBBone::getTweenPivot() const
{
    return this->_tweenPivot;
}

int DBBone::getNeedUpdate()
{
    return this->_needUpdate;
}

void DBBone::calculateRelativeParentTransform()
{
    global.scaleX = origin.scaleX * _tween.scaleX * offset.scaleX;
    global.scaleY = origin.scaleY * _tween.scaleY * offset.scaleY;
    global.skewX = origin.skewX + _tween.skewX + offset.skewX;
    global.skewY = origin.skewY + _tween.skewY + offset.skewY;
    global.x = origin.x + _tween.x + offset.x;
    global.y = origin.y + _tween.y + offset.y;
}

// static method
bool DBBone::sortState(const DBTimelineState *a, const DBTimelineState *b)
{
    return a->getAnimationState()->getLayer() < b->getAnimationState()->getLayer();
}

void DBBone::addState(DBTimelineState *timelineState)
{
    auto iterator = std::find(_timelineStateList.cbegin(), _timelineStateList.cend(), timelineState);
    
    if (iterator == _timelineStateList.cend())
    {
        _timelineStateList.push_back(timelineState);
        std::sort(_timelineStateList.begin() , _timelineStateList.end() , sortState);
    }
}

void DBBone::removeState(DBTimelineState *timelineState)
{
    auto iterator = std::find(_timelineStateList.begin(), _timelineStateList.end(), timelineState);
    
    if (iterator != _timelineStateList.end())
    {
        _timelineStateList.erase(iterator);
    }
}

void DBBone::invalidUpdate()
{
    _needUpdate = 2;
}


void DBBone::arriveAtFrame(TransformFrame *frame, const DBTimelineState *timelineState, DBAnimationState *animationState, bool isCross)
{

    // modify by Relvin need todo
    bool displayControl =
    animationState->displayControl &&
    (displayController.empty() || displayController == animationState->name) &&
    animationState->getMixingTransform(this->getName());
    
    // && timelineState->_weight > 0
    // TODO: 需要修正混合动画干扰关键帧数据的问题，如何正确高效的判断混合动画？
    if (displayControl)
    {
        #if 0
        
        if (!frame->event.empty() && _armature->_eventDispatcher->hasEvent(EventData::EventType::BONE_FRAME_EVENT))
        {
            EventData *eventData = EventData::borrowObject(EventData::EventType::BONE_FRAME_EVENT);
            eventData->armature = _armature;
            eventData->bone = this;
            eventData->animationState = animationState;
            eventData->frameLabel = frame->event;
            eventData->frame = frame;
            _armature->_eventDataList.push_back(eventData);
        }
        
        if (!frame->sound.empty() && Armature::soundEventDispatcher && Armature::soundEventDispatcher->hasEvent(EventData::EventType::SOUND))
        {
            EventData *eventData = EventData::borrowObject(EventData::EventType::SOUND);
            eventData->armature = _armature;
            eventData->bone = this;
            eventData->animationState = animationState;
            eventData->sound = frame->sound;
            Armature::soundEventDispatcher->dispatchEvent(eventData);
            EventData::returnObject(eventData);
        }
        
        if (!frame->action.empty())
        {
            for (size_t i = 0, l = _slotList.size(); i < l; ++i)
            {
                if (_slotList[i]->_childArmature)
                {
                    _slotList[i]->_childArmature->_animation->gotoAndPlay(frame->action);
                }
            }
        }
        #endif
    }

}


NAME_SPACE_DRAGON_BONES_END