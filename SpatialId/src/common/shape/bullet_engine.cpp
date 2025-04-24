/**
 * @file bullet_engine.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "bullet_engine.h"

#include <cmath>

namespace spatialid {

// 衝突判定 contactPairTest のコールバック関数
struct Bullet2ContactResultCallback
    : public btCollisionWorld::ContactResultCallback {
    bool hit;
    Bullet2ContactResultCallback() : hit(false) {}
    virtual btScalar addSingleResult(
        btManifoldPoint &cp, const btCollisionObjectWrapper *col_obj0_wrap,
        int partId0, int index0, const btCollisionObjectWrapper *col_obj1_wrap,
        int partId1, int index1) {
        hit = (cp.m_distance1 <= 0.f);
        return 1.f;
    }
};

// コンストラクタ
BulletEngine::BulletEngine()
    : collision_configuration_(nullptr),
      dispatcher_(nullptr),
      overlapping_pair_cache_(nullptr),
      solver_(nullptr),
      dynamics_world_(nullptr) {}

// デストラクタ
BulletEngine::~BulletEngine() { destroyWorld(); }

// ２つのオブジェクトの衝突判定を行う
bool BulletEngine::hitTest(BulletObject &obj_a, BulletObject &obj_b) {
    if (obj_a.geom_type == obj_b.geom_type) {
        return false;
    }

    if (!dynamics_world_) {
        createWorld();
    }

    btCollisionShape *shape_a = createCollisionShape(obj_a);
    btRigidBody *body_a = nullptr;
    if (shape_a) {
        body_a = createRigidBody(shape_a, obj_a);
    }

    btCollisionShape *shape_b = createCollisionShape(obj_b);
    btRigidBody *body_b = nullptr;
    if (shape_b) {
        body_b = createRigidBody(shape_b, obj_b);
    }

    bool hit = false;
    if (body_a && body_b) {
        dynamics_world_->addRigidBody(body_a);
        dynamics_world_->addRigidBody(body_b);

        Bullet2ContactResultCallback callback;
        dynamics_world_->contactPairTest(body_a, body_b, callback);
        hit = callback.hit;

        // body_a と body_b は deleteObject() 内で解放される
        deleteObject();
    } else {
        if (body_a) {
            delete body_a;
        }
        if (body_b) {
            delete body_b;
        }
    }

    if (shape_a) {
        delete shape_a;
    }
    if (shape_b) {
        delete shape_b;
    }

    return hit;
}

/**
 * @brief 仮想空間を作成する
 *
 */
void BulletEngine::createWorld() {
    destroyWorld();

    collision_configuration_ = new btDefaultCollisionConfiguration();
    dispatcher_ = new btCollisionDispatcher(collision_configuration_);
    overlapping_pair_cache_ = new btDbvtBroadphase();
    solver_ = new btSequentialImpulseConstraintSolver();
    dynamics_world_ = new btDiscreteDynamicsWorld(
        dispatcher_, overlapping_pair_cache_, solver_, collision_configuration_);
    dynamics_world_->setGravity(btVector3(0, 0, 0));
}

/**
 * @brief btCollisionShapeを作成する
 *
 * @param obj
 * @return btCollisionShape*
 */
btCollisionShape *BulletEngine::createCollisionShape(BulletObject &obj) {
    switch (obj.geom_type) {
        case GeomType::kGeomBox:
            return new btBoxShape(btVector3(btScalar(obj.half_extents[0]),
                                            btScalar(obj.half_extents[1]),
                                            btScalar(obj.half_extents[2])));
            break;
        case GeomType::kGeomCapsule:
            return new btCapsuleShape(btScalar(obj.radius),
                                      btScalar(obj.height));
            break;
        case GeomType::kGeomCylinder:
            return new btCylinderShapeZ(btVector3(btScalar(obj.radius),
                                                 btScalar(obj.radius),
                                                 btScalar(obj.height)));
            break;
        case GeomType::kGeomSphere:
            return new btSphereShape(btScalar(obj.radius));
            break;
        default:
            break;
    }
    return nullptr;
}

/**
 * @brief btRigidBodyを作成する
 *
 * @param obj
 * @return btRigidBody*
 */
btRigidBody *BulletEngine::createRigidBody(btCollisionShape *shape,
                                           BulletObject &obj) {
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(btScalar(obj.center_point[0]),
                                  btScalar(obj.center_point[1]),
                                  btScalar(obj.center_point[2])));

    if (obj.geom_type == GeomType::kGeomCapsule ||
        obj.geom_type == GeomType::kGeomCylinder) {
        transform.setRotation(btQuaternion(
            btScalar(obj.orientations[0]), btScalar(obj.orientations[1]),
            btScalar(obj.orientations[2]), btScalar(obj.orientations[3])));
    }

    btScalar mass(0.f);
    btVector3 localInertia(0, 0, 0);
    btDefaultMotionState *myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape,
                                                    localInertia);
    return new btRigidBody(rbInfo);
}

/**
 * @brief 仮想空間からオブジェクトを削除する
 *
 */
void BulletEngine::deleteObject() {
    if (dynamics_world_) {
        for (int i = dynamics_world_->getNumCollisionObjects() - 1; i >= 0;
             i--) {
            btCollisionObject *obj =
                dynamics_world_->getCollisionObjectArray()[i];
            btRigidBody *body = btRigidBody::upcast(obj);
            if (body && body->getMotionState()) {
                delete body->getMotionState();
            }
            dynamics_world_->removeCollisionObject(obj);
            delete obj;
        }
    }
}

/**
 * @brief 仮想空間を破棄する
 *
 */
void BulletEngine::destroyWorld() {
    if (dynamics_world_) {
        deleteObject();
        delete dynamics_world_;
        dynamics_world_ = nullptr;
    }
    if (solver_) {
        delete solver_;
        solver_ = nullptr;
    }
    if (overlapping_pair_cache_) {
        delete overlapping_pair_cache_;
        overlapping_pair_cache_ = nullptr;
    }
    if (dispatcher_) {
        delete dispatcher_;
        dispatcher_ = nullptr;
    }
    if (collision_configuration_) {
        delete collision_configuration_;
        collision_configuration_ = nullptr;
    }
}

}  // namespace spatialid
