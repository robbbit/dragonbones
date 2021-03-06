//
//  DBBone.hpp
//  dragonbones
//
//  Created by Relvin on 16/2/4.
//  Copyright © 2016年 Relvin. All rights reserved.
//

#ifndef DBBone_hpp
#define DBBone_hpp

#include "2d/CCNode.h"
#include "DragonBones.h"
#include "objects/BoneData.h"
#include "core/DBBase.h"
#include "animation/DBTimelineState.h"
#include "objects/TransformFrame.h"

NAME_SPACE_DRAGON_BONES_BEGIN

class DBArmature;
class DBSlot;
class DBIKConstraint;

class DBBone
: public cocos2d::Node
, public DBBase
{
public:
    static DBBone* create(BoneData* boneData);
    bool initWithBoneData(BoneData* boneData);
    DBBone();
    virtual ~DBBone();
    
    void addChildBone(DBBone *child);
    void removeChildBone(DBBone *bone, bool recursion);
    
    void setArmature(DBArmature *armature);
    void calculateParentTransform( Transform &transform, Matrix &matrix ) override;
    
    virtual void calculateRelativeParentTransform() override;
    
    void update(bool isFading);
    void blendingTimeline();
    
    
    virtual cocos2d::Vector<DBBone*>& getBones() { return _childBones; }
    virtual const cocos2d::Vector<DBBone*>& getBones() const { return _childBones; }
    
    void arriveAtFrame(TransformFrame *frame, const DBTimelineState *timelineState, DBAnimationState *animationState, bool isCross);
    virtual const Point& getTweenPivot() const;
    int getNeedUpdate();
    
    static bool sortState(const DBTimelineState *a, const DBTimelineState *b);
    void addState(DBTimelineState *timelineState);
    void removeState(DBTimelineState *timelineState);
    void invalidUpdate();
    
    void addSlot(DBSlot *slot);
    void removeSlot(DBSlot *slot);
    
    void hideSlots();
    inline float getLength() {return this->_length;};
    
    void adjustGlobalTransformMatrixByIK();
    inline float getRotationIK () {return this->_rotationIK;};
    void setRotationIK(float rotationIK);
    inline bool getIsIKConstraint() {return this->_isIKConstraint;};
    void setIsIKConstraint(bool isIKConstraint);
    void setIkConstraint(DBIKConstraint *ikConstraint);
    void operationInvalidUpdate(DBBone* bone);
    virtual void updateGlobal(Transform &transform, Matrix &matrix) override;
    float getParentBoneRotation();
private:
    
    BoneData* _boneData;
    DBArmature* _pArmature;
    
    std::string displayController;
    bool applyOffsetTranslationToChild;
    bool applyOffsetRotationToChild;
    bool applyOffsetScaleToChild;
    
    Transform _globalTransformForChild;
    Matrix _globalTransformMatrixForChild;
    

    cocos2d::Vector<DBBone*> _childBones;        ///< array of children nodes
    cocos2d::Vector<DBSlot*> _slotList;
    cocos2d::Vector<DBTimelineState*> _timelineStateList;
    
    int _needUpdate;
    bool _isColorChanged;
    
    Point _tweenPivot;
    Transform _tween;
    

    Transform _tempGlobalTransformForChild;
    Matrix _tempGlobalTransformMatrixForChild;
    float _length;
    float _rotationIK;
    bool _isIKConstraint;
    
    DBIKConstraint *_ikConstraint;

    
};

NAME_SPACE_DRAGON_BONES_END


#endif /* DBBone_hpp */
