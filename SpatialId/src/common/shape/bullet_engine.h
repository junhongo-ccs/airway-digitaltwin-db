/**
 * @file bullet_engine.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __BULLET_ENGINE_H__
#define __BULLET_ENGINE_H__

#include <tuple>

#include "btBulletDynamicsCommon.h"
#include "common/object/point.h"
#include "common/shape/bullet_object.h"

namespace spatialid {

/**
 * @brief 衝突判定エンジン(bullet3)
 * 
 */
class BulletEngine {
   private:
    btDefaultCollisionConfiguration *collision_configuration_;
    btCollisionDispatcher *dispatcher_;
    btBroadphaseInterface *overlapping_pair_cache_;
    btSequentialImpulseConstraintSolver *solver_;
    btDiscreteDynamicsWorld *dynamics_world_;

   public:
    /**
     * @brief Construct a new Bullet Engine object
     *
     */
    BulletEngine();

    /**
     * @brief Destroy the Bullet Engine object
     *
     */
    virtual ~BulletEngine();

    /**
     * @brief ２つのオブジェクトの衝突判定を行う
     *
     * @param objA オブジェクトA
     * @param objB オブジェクトB(ただし、オブジェクトAと同じ種類は指定不可)
     * @return true
     * @return false
     */
    bool hitTest(BulletObject &objA, BulletObject &objB);

   private:
    void createWorld();
    void destroyWorld();
    btCollisionShape *createCollisionShape(BulletObject &obj);
    btRigidBody *createRigidBody(btCollisionShape *shape, BulletObject &obj);
    void deleteObject();
};

}  // namespace spatialid

#endif  // __BULLET_ENGINE_H__
