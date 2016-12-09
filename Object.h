/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#ifndef OBJECT_H_
#define OBJECT_H_
#pragma once

#include <glew/glew.h>
#include <SDL_opengl.h>

#include "Matrix.h"
#include "Realm.h"

class SheetContainer;
class SheetRect;

namespace AOD {
  class Collision_Info;

  // -------------- OBJECT ----------------------------------------------------

  class Object {
  private:
    void Refresh_Transform();
  public:
    enum class Type { Circle, AABB, Polygon, Ray, nil };
  protected:
    GLuint image;
    int ID;
    float rotation,
          rotation_velocity;
    Matrix matrix;
    Type type;
    Vector position,
           velocity, scale,
           size,
           image_size,
           rotate_origin;
    int layer;
    float alpha;
    bool flipped_x, flipped_y;
    GLfloat _UV[8];
    friend AOD_Engine::Realm; // to access layer
    bool is_coloured, visible, static_pos;
    float red, green, blue;

    bool transformed;
  public:
    Object(Type = Type::nil);
    inline void Set_ID(int id) { ID = id; }
    inline int Ret_ID() const { return ID; }
    void Set_Position(float, float);
    void Set_Position(const Vector&);
    void Add_Position(float, float);
    void Add_Position(const Vector&);
    const Vector& R_Position() const;

    void Set_Sprite(GLuint, bool reset_size = 0);
    void Set_Sprite(const SheetContainer&);
    void Set_Sprite(const SheetRect&);
    GLuint R_Sprite_Texture();

    void Set_Rotation(float);
    float R_Rotation();

    void Apply_Force( const Vector&);
    void Set_Velocity(const Vector&);
    void Set_Velocity(float x, float y);
    void Set_Torque(float);
    Vector R_Velocity() const;
    float R_Torque();

    void Set_Sprite_Frame(float left_x,  float top_y,
                          float right_x, float bot_y);

    void Set_UVs(const Vector& left, const Vector& right, bool reset_flip = 1);
    void R_UVs(Vector& left, Vector& right);
    void Flip_X();
    void Flip_Y();

    void Set_Size(const Vector&, bool scale_image = 0);
    void Set_Size(int x, int y, bool scale_image = 0);
    void Set_Image_Size(const Vector&);
    void Set_Visible(bool);

    Vector R_Size();
    Vector R_Img_Size();

    void Set_Colour(float red = 1, float green = 1,
                   float blue = 1, float alpha = 1);
    void Cancel_Colour();
    void Set_Is_Static_Pos(bool);

    void Set_Origin(const AOD::Vector&);
    // will reset origin to i mage_size/2
    void Clear_Origin();
    inline const AOD::Vector& R_Origin() const { return rotate_origin; }

    float R_Green();
    float R_Red();
    float R_Blue();
    float R_Alpha();
    bool R_Is_Coloured();
    bool R_Is_Visible();
    bool R_Is_Static_Pos();
    inline bool R_Flipped_X() const { return flipped_x; }
    inline bool R_Flipped_Y() const { return flipped_y; }

    inline const Type& R_Type() const { return type; }
    inline const Matrix& R_Matrix() const { return matrix; }
    // ---- utility ----
    virtual void Update();
    Collision_Info Collision(Object*);

    static const float Vertices[8];
  };

  // -------------- POLY OBJ --------------------------------------------------

  using AODVecs = std::vector<AOD::Vector>;
  class AABBObj;

  class PolyObj : public Object {
  protected:
    AODVecs vertices, vertices_transform;
    void Build_Transform();
  public:
    PolyObj(); // empty poly
    PolyObj(const AODVecs& vertices, AOD::Vector off = {0,0});

    // ---- ret/set ----
    // will override previous vectors
    void Set_Vertices(const AODVecs& , bool reorder = 1);
    const AODVecs& R_Vertices() const;
    const AODVecs& R_Transformed_Vertices(bool force = 0);

    // ---- utility ----

    // Returns information on current collision state with another poly
    Collision_Info Collide(PolyObj* poly, AOD::Vector velocity);
    Collision_Info Collide(AABBObj* aabb, AOD::Vector velocity);
  };

  class AABBObj : public PolyObj {
  public:
    AABBObj(AOD::Vector size = {0,0});
    AABBObj(AOD::Vector size = {0,0}, AOD::Vector pos = {0,0});

    // ---- utility ----

    // Returns information on current collision with an AABB
    Collision_Info Collide(AABBObj* aabb, AOD::Vector velocity);
    Collision_Info Collide(PolyObj* poly, AOD::Vector velocity);
  };

  // Valuable information from a collision, "translation"
  // could mean different things dependent on the collision type
  class Collision_Info {
  public:
    bool collision,
         will_collide;
    AOD::Vector translation,
                projection, normal;
    AOD::PolyObj* obj;
    // collision will always be true in def constructor
    Collision_Info();
    Collision_Info(bool collision);
    Collision_Info(const AOD::Vector&, bool collision,
                                       bool will_collide);
  };
}

#endif
