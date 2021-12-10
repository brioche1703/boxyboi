#pragma once

#include <Box2D\Box2D.h>
#include "IndexedTriangleList.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat2.h"
#include "Colors.h"
#include "Pipeline.h"
#include "SolidEffect.h"
#include "BodyPtr.h"
#include "Boundaries.h"
#include <random>

class Box
{
public:
	class ColorTrait
	{
	public:
		virtual ~ColorTrait() = default;
		virtual Color GetColor() const = 0;
		virtual std::unique_ptr<ColorTrait> Clone() const = 0;
	};
public:
	static std::unique_ptr<Box> Box::Spawn( float size,const Boundaries& bounds,b2World& world,std::mt19937& rng );
	std::unique_ptr<Box> Spawn(float size, const Boundaries& bounds, b2World& world, Box& pBox, Vec2& pos);
	Box( std::unique_ptr<ColorTrait> pColorTrait, b2World& world,const Vec2& pos,
		float size = 1.0f,float angle = 0.0f,Vec2 linVel = {0.0f,0.0f},float angVel = 0.0f )
		:
		pColorTrait( std::move( pColorTrait ) ),
		size( size )
	{
		Init();
		{
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;
			bodyDef.position = b2Vec2( pos );
			bodyDef.linearVelocity = b2Vec2( linVel );
			bodyDef.angularVelocity = angVel;
			bodyDef.angle = angle;
			pBody = BodyPtr::Make( world,bodyDef );
		}
		{
			b2PolygonShape dynamicBox;
			dynamicBox.SetAsBox( size,size );
			b2FixtureDef fixtureDef;
			fixtureDef.shape = &dynamicBox;
			fixtureDef.density = 1.0f;
			fixtureDef.friction = 0.0f;
			fixtureDef.restitution = 1.0f;
			pBody->CreateFixture( &fixtureDef );
		}
		pBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
	}
	void Draw( Pipeline<SolidEffect>& pepe ) const
	{
		pepe.effect.vs.BindTranslation( GetPosition() );
		pepe.effect.vs.BindRotation( Mat2::Rotation( GetAngle() ) * Mat2::Scaling( GetSize() ) );
		pepe.effect.ps.BindColor( GetColorTrait().GetColor() );
		pepe.Draw( model );
	}
	void ApplyLinearImpulse( const Vec2& impulse )
	{
		pBody->ApplyLinearImpulse( (b2Vec2)impulse,(b2Vec2)GetPosition(),true );
	}
	void ApplyAngularImpulse( float impulse )
	{
		pBody->ApplyAngularImpulse( impulse,true );
	}
	float GetAngle() const
	{
		return pBody->GetAngle();
	}
	Vec2 GetPosition() const
	{
		return (Vec2)pBody->GetPosition();
	}
	float GetAngularVelocity() const
	{
		return pBody->GetAngularVelocity();
	}
	Vec2 GetVelocity() const
	{
		return (Vec2)pBody->GetLinearVelocity();
	}
	float GetSize() const
	{
		return size;
	}
	void SetDestroyed(bool value) {
		destroyed = value;
	}
	bool IsDestroyed() const
	{
		return destroyed;
	}
	const ColorTrait& GetColorTrait() const
	{
		return *pColorTrait;
	}
	void SetSplit(bool value) {
		split = value;
	}
	bool IsSplit() const {
		return split;
	}
	std::vector<std::unique_ptr<Box>> GetSplits(const Boundaries& bounds,b2World& world) {
		if (size >= minSize) {
			std::vector<std::unique_ptr<Box>> newBoxes;
			const auto pos = Vec2{ GetPosition().x, GetPosition().y };
			const auto newSize = size /= 2.0f;

			Vec2 offsets[4] = {
				{pos.x - (size / 4.0f) - 0.1f, pos.y - (size / 4.0f) - 0.1f},
				{pos.x - (size / 4.0f) - 0.1f, pos.y + (size / 4.0f) + 0.1f},
				{pos.x + (size / 4.0f) + 0.1f, pos.y - (size / 4.0f) - 0.1f},
				{pos.x + (size / 4.0f) + 0.1f, pos.y + (size / 4.0f) + 0.1f},
			};

			newBoxes.emplace_back(Box::Spawn(newSize, bounds, world, *this, offsets[0]));
			newBoxes.emplace_back(Box::Spawn(newSize, bounds, world, *this, offsets[1]));
			newBoxes.emplace_back(Box::Spawn(newSize, bounds, world, *this, offsets[2]));
			newBoxes.emplace_back(Box::Spawn(newSize, bounds, world, *this, offsets[3]));

			return newBoxes;
		}
		return {};
	}

private:
	static void Init()
	{
		if( model.indices.size() == 0 )
		{
			model.vertices = { { -1.0f,-1.0 },{ 1.0f,-1.0 },{ -1.0f,1.0 },{ 1.0f,1.0 } };
			model.indices = { 0,1,2, 1,2,3 };
		}
	}
private:
	static constexpr float minSize = 0.2f;
	static IndexedTriangleList<Vec2> model;
	float size;
	BodyPtr pBody;
	std::unique_ptr<ColorTrait> pColorTrait;
	bool destroyed = false;
	bool split = false;
};