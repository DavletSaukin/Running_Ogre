#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D\Graphics/Animation.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D\Graphics\BillboardSet.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <random>

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"
#include "Shooting.h"
#include "Mover.h"

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    viewFromAbove_(true)
{
    // Зарегистрируем фабрику и атрибуты для компонента Character, чтобы он мог быть создан с помощью CreateComponent и загружен / сохранен
    Character::RegisterObject(context);
	context->RegisterFactory<Shooting>();
	context->RegisterFactory<Mover>();
}

CharacterDemo::~CharacterDemo()
{
}

void CharacterDemo::Start()
{
    // Выполнение запуска базового класса
    Sample::Start();
    if (touchEnabled_)
        touch_ = new Touch(context_, TOUCH_SENSITIVITY);

    // Создание содержимого статической сцены
    CreateScene();

    // Создание управляемого персонажа
    CreateCharacter();

    // Создание UI
    CreateInstructions();

    // Подпиcка на необходимые события
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_RELATIVE);
}

void CharacterDemo::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Создание компонентов подсистемы сцены
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<PhysicsWorld>();

    //Создание камеры
    cameraNode_ = new Node(context_);
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

    // Создание сцены
    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

    // Создание источника света
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);

    // Создание пола
    Node* floorNode = scene_->CreateChild("Floor");
    floorNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    floorNode->SetScale(Vector3(200.0f, 1.0f, 200.0f));
    StaticModel* object = floorNode->CreateComponent<StaticModel>();
    object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

    RigidBody* body = floorNode->CreateComponent<RigidBody>();
    // Use collision layer bit 2 to mark world scenery. This is what we will raycast against to prevent camera from going
    // inside geometry
    body->SetCollisionLayer(1);
    CollisionShape* shape = floorNode->CreateComponent<CollisionShape>();
    shape->SetBox(Vector3::ONE);

    //создаём стены

	float boxScale = 4.0f;

	//стена без пушек
    for (unsigned int i = 0; i < 23; i++)
    {
		Node* boxNode = scene_->CreateChild("Box");
		boxNode->SetPosition(Vector3(boxScale * i, 2.5f, boxScale));
		boxNode->SetScale(boxScale);
		StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
		boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
		boxObject->SetCastShadows(true);  
		boxObject->SetOccluder(false);

		RigidBody* body = boxNode->CreateComponent<RigidBody>();
		body->SetCollisionLayer(1);
		body->SetMass(boxScale * 100.0f);
		CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
		shape->SetBox(Vector3::ONE);
    }

	//создаём блок
	Node* firstboxNode = scene_->CreateChild("Box");
	firstboxNode->SetPosition(Vector3(0, 2.5f, 0));
	firstboxNode->SetScale(boxScale);
	StaticModel* firstboxObject = firstboxNode->CreateComponent<StaticModel>();
	firstboxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	firstboxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	firstboxObject->SetCastShadows(true);
	firstboxObject->SetOccluder(false);

	RigidBody* fbody = firstboxNode->CreateComponent<RigidBody>();
	fbody->SetCollisionLayer(1);
	fbody->SetMass(boxScale * 100.0f);
	CollisionShape* fshape = firstboxNode->CreateComponent<CollisionShape>();
	fshape->SetBox(Vector3::ONE);
	
	//создаём блок
	Node* secboxNode = scene_->CreateChild("Box");
	secboxNode->SetPosition(Vector3(boxScale * 24, 2.5f, -boxScale * 23));
	secboxNode->SetScale(boxScale);
	StaticModel* secboxObject = secboxNode->CreateComponent<StaticModel>();
	secboxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	secboxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	secboxObject->SetCastShadows(true);
	secboxObject->SetOccluder(false);

	RigidBody* secbody = secboxNode->CreateComponent<RigidBody>();
	secbody->SetCollisionLayer(1);
	secbody->SetMass(boxScale * 100.0f);
	CollisionShape* sshape = secboxNode->CreateComponent<CollisionShape>();
	sshape->SetBox(Vector3::ONE);

	//создаём первую пушку
	float scale = 1.5f;

	Node* fcannonNode = scene_->CreateChild("Cannon");
	fcannonNode->SetPosition(Vector3(boxScale * 23, 2.5f, boxScale + 2.0f));
	fcannonNode->SetRotation(Quaternion(0.0f, -90.0f, 90.0f));
	fcannonNode->SetScale(scale);

	AnimatedModel* fcobject = fcannonNode->CreateComponent<AnimatedModel>();
	fcobject->SetModel(cache->GetResource<Model>("Models/Cannon.mdl"));
	fcobject->SetMaterial(cache->GetResource<Material>("Materials/Cannon.xml"));
	fcobject->SetCastShadows(true);
	fcobject->SetOccluder(false);

	RigidBody* fcbody = fcannonNode->CreateComponent<RigidBody>();
	fcbody->SetCollisionLayer(1);
	fcbody->SetMass(scale * 300.0f);
	fcbody->SetFriction(1);
	fcbody->SetRollingFriction(1);

	RigidBody* fcannonballPLaceBody = fcannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->CreateComponent<RigidBody>();
	fcannonballPLaceBody->SetFriction(1);
	fcannonballPLaceBody->SetRollingFriction(1);

	CollisionShape* fcshape = fcannonNode->CreateComponent<CollisionShape>();
	fcshape->SetCapsule(2.0f, 6.0f);

	// и ядро в ней
	Node* fcannonballNode = scene_->CreateChild("Cannonball");
	const Vector3 fcpos = fcannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->GetWorldPosition();
	fcannonballNode->SetPosition(fcpos);
	fcannonballNode->SetScale(2.0f);

	StaticModel* fcannonballObject = fcannonballNode->CreateComponent<StaticModel>();
	fcannonballObject->SetModel(cache->GetResource<Model>("Models/Cannonball.mdl"));
	fcannonballObject->SetMaterial(cache->GetResource<Material>("Materials/Cannonball.xml"));
	fcannonballObject->SetCastShadows(true);
	fcannonballObject->SetOccluder(false);

	RigidBody* fcannonballBody = fcannonballNode->CreateComponent<RigidBody>();
	fcannonballBody->SetCollisionLayer(1.0f);
	fcannonballBody->SetMass(2.0f);
	fcannonballBody->SetFriction(1);
	fcannonballBody->SetRollingFriction(1.0);

	CollisionShape* fcannonballShape = fcannonballNode->CreateComponent<CollisionShape>();
	fcannonballShape->SetSphere(0.8f);

	Shooting* fshoot = fcannonballNode->CreateComponent<Shooting>();
	fshoot->SetParameters(fcpos, Vector3::BACK);

	//противоположная стена без пушек
	for (unsigned int i = 0; i < 25; i++)
	{
		Node* boxNode = scene_->CreateChild("Box");
		boxNode->SetPosition(Vector3(boxScale * i, 2.5f, -boxScale * 24));
		boxNode->SetScale(boxScale);
		StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
		boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
		boxObject->SetCastShadows(true);
		boxObject->SetOccluder(false);

		RigidBody* body = boxNode->CreateComponent<RigidBody>();
		body->SetCollisionLayer(1);
		body->SetMass(boxScale * 100.0f);
		CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
		shape->SetBox(Vector3::ONE);
	}

	//создаём вторую пушку
	Node* scannonNode = scene_->CreateChild("Cannon");
	scannonNode->SetPosition(Vector3(-3.0f, 2.5f, -boxScale * 23));
	scannonNode->SetRotation(Quaternion(-90.0f, -90.0f, 0.0f));
	scannonNode->SetScale(scale);

	AnimatedModel* scobject = scannonNode->CreateComponent<AnimatedModel>();
	scobject->SetModel(cache->GetResource<Model>("Models/Cannon.mdl"));
	scobject->SetMaterial(cache->GetResource<Material>("Materials/Cannon.xml"));
	scobject->SetCastShadows(true);
	scobject->SetOccluder(false);

	RigidBody* scbody = scannonNode->CreateComponent<RigidBody>();
	scbody->SetCollisionLayer(1);
	scbody->SetMass(scale * 300.0f);
	scbody->SetFriction(1);
	scbody->SetRollingFriction(1);

	RigidBody* scannonballPLaceBody = scannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->CreateComponent<RigidBody>();
	scannonballPLaceBody->SetFriction(1);
	scannonballPLaceBody->SetRollingFriction(1);

	CollisionShape* scshape = scannonNode->CreateComponent<CollisionShape>();
	scshape->SetCapsule(2.0f, 6.0f);

	// и ядро в ней
	Node* scannonballNode = scene_->CreateChild("Cannonball");
	const Vector3 scpos = scannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->GetWorldPosition();
	scannonballNode->SetPosition(scpos);
	scannonballNode->SetScale(2.0f);

	StaticModel* scannonballObject = scannonballNode->CreateComponent<StaticModel>();
	scannonballObject->SetModel(cache->GetResource<Model>("Models/Cannonball.mdl"));
	scannonballObject->SetMaterial(cache->GetResource<Material>("Materials/Cannonball.xml"));
	scannonballObject->SetCastShadows(true);
	scannonballObject->SetOccluder(false);

	RigidBody* scannonballBody = scannonballNode->CreateComponent<RigidBody>();
	scannonballBody->SetCollisionLayer(1.0f);
	scannonballBody->SetMass(2.0f);
	scannonballBody->SetFriction(1);
	scannonballBody->SetRollingFriction(1.0);

	CollisionShape* scannonballShape = scannonballNode->CreateComponent<CollisionShape>();
	scannonballShape->SetSphere(0.8f);

	Shooting* sshoot = scannonballNode->CreateComponent<Shooting>();
	sshoot->SetParameters(scpos, Vector3::RIGHT);

	//конец создания стен

	/*
	Создаём лабиринт. 
	Используем упрощённый алгоритм на основе
	алгоритма Эллера
	*/

	Vector3 x_OrientedWallScale = Vector3(boxScale, boxScale, 0.5f);
	Vector3 z_OrientedWallScale = Vector3(0.5f, boxScale, boxScale); 

	std::random_device rd;
	std::mt19937 gen(rd());
	std::bernoulli_distribution d(0.5); //вероятность 50/50

	for (int i = 0; i < 23; i++)
	{
		int x_OrientedWallCount = 0;
		int z_OrientedWallCount = 0;
		bool isEmpty = true; //флаг наличия пушки/блока в правой стене в текущей итерации
		bool hasNinja = false; //флаг наличия ниндзи в коридоре

		for (int j = 0; j < 22; ++j)
		{
			Node* boxNode = scene_->CreateChild("Wall");

			if (d(gen) == 1)
			{
				boxNode->SetPosition(Vector3(boxScale * j + boxScale + 1.7f, 2.5f, -boxScale * i));
				boxNode->SetScale(z_OrientedWallScale);
				x_OrientedWallCount = 0;
				z_OrientedWallCount++;
				hasNinja = true;
			}
			else 
			{
				boxNode->SetPosition(Vector3(boxScale * j + boxScale, 2.5f, -boxScale * i - 1.7f));
				boxNode->SetScale(x_OrientedWallScale);
				x_OrientedWallCount++;
			}

			StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
			boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
			boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
			boxObject->SetCastShadows(true);
			boxObject->SetOccluder(false);

			RigidBody* body = boxNode->CreateComponent<RigidBody>();
			body->SetCollisionLayer(1);
			body->SetMass(boxScale * 100.0f);
			CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
			shape->SetBox(Vector3::ONE);

			//формируем левую стену
			if (j == 21 && x_OrientedWallCount > 3)
			{
				/*
				Если от левой стены в сторону правой
				имеется коридор длинной более трёх  блоков,
				создём пушку
				*/
				
				float scale = 1.5f;

				Node* cannonNode = scene_->CreateChild("Cannon");
				cannonNode->SetPosition(Vector3(boxScale * 24 + 2.0f, 2.5f, -boxScale * i));
				cannonNode->SetRotation(Quaternion(90.0f, -90.0f, 0.0f));
				cannonNode->SetScale(scale);

				AnimatedModel* object = cannonNode->CreateComponent<AnimatedModel>();
				object->SetModel(cache->GetResource<Model>("Models/Cannon.mdl"));
				object->SetMaterial(cache->GetResource<Material>("Materials/Cannon.xml"));
				object->SetCastShadows(true);
				object->SetOccluder(false);

				RigidBody* body = cannonNode->CreateComponent<RigidBody>();
				body->SetCollisionLayer(1);
				body->SetMass(scale * 300.0f);
				body->SetFriction(1);
				body->SetRollingFriction(1);

				RigidBody* cannonballPLaceBody = cannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->CreateComponent<RigidBody>();
				cannonballPLaceBody->SetFriction(1);
				cannonballPLaceBody->SetRollingFriction(1);

				CollisionShape* shape = cannonNode->CreateComponent<CollisionShape>();
				shape->SetCapsule(2.0f, 6.0f);

				// и ядро в ней
				Node* cannonballNode = scene_->CreateChild("Cannonball");
				const Vector3 pos = cannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->GetWorldPosition();
				cannonballNode->SetPosition(pos);
				cannonballNode->SetScale(2.0f);

				StaticModel* cannonballObject = cannonballNode->CreateComponent<StaticModel>();
				cannonballObject->SetModel(cache->GetResource<Model>("Models/Cannonball.mdl"));
				cannonballObject->SetMaterial(cache->GetResource<Material>("Materials/Cannonball.xml"));
				cannonballObject->SetCastShadows(true);
				cannonballObject->SetOccluder(false);

				RigidBody* cannonballBody = cannonballNode->CreateComponent<RigidBody>();
				cannonballBody->SetCollisionLayer(1.0f);
				cannonballBody->SetMass(2.0f);
				cannonballBody->SetFriction(1);
				cannonballBody->SetRollingFriction(1.0);

				CollisionShape* cannonballShape = cannonballNode->CreateComponent<CollisionShape>();
				cannonballShape->SetSphere(0.8f);

				Shooting* shoot = cannonballNode->CreateComponent<Shooting>();
				shoot->SetParameters(pos, Vector3::LEFT);
			}
			else if (j == 21 && x_OrientedWallCount <= 3)
			{
				//создаём блок

				Node* boxNode = scene_->CreateChild("Box");
				boxNode->SetPosition(Vector3(boxScale * 24, 2.5f, -boxScale * i));
				boxNode->SetScale(boxScale);
				StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
				boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
				boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
				boxObject->SetCastShadows(true);
				boxObject->SetOccluder(false);

				RigidBody* body = boxNode->CreateComponent<RigidBody>();
				body->SetCollisionLayer(1);
				body->SetMass(boxScale * 100.0f);
				CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
				shape->SetBox(Vector3::ONE);
			}

			//формируем правую стену 
			if (i != 0 && z_OrientedWallCount == 0 && x_OrientedWallCount > 3 && isEmpty == true)
			{
				/*
				Если от правой стены в сторону левой
				имеется коридор длинной более трёх  блоков,
				создём пушку
				*/
				
				float scale = 1.5f;

				Node* cannonNode = scene_->CreateChild("Cannon");
				cannonNode->SetPosition(Vector3(-3.0f, 2.5f, -boxScale * i));
				cannonNode->SetRotation(Quaternion(-90.0f, -90.0f, 0.0f));
				cannonNode->SetScale(scale);

				AnimatedModel* object = cannonNode->CreateComponent<AnimatedModel>();
				object->SetModel(cache->GetResource<Model>("Models/Cannon.mdl"));
				object->SetMaterial(cache->GetResource<Material>("Materials/Cannon.xml"));
				object->SetCastShadows(true);
				object->SetOccluder(false);

				RigidBody* body = cannonNode->CreateComponent<RigidBody>();
				body->SetCollisionLayer(1);
				body->SetMass(scale * 300.0f);
				body->SetFriction(1);
				body->SetRollingFriction(1);

				RigidBody* cannonballPLaceBody = cannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->CreateComponent<RigidBody>();
				cannonballPLaceBody->SetFriction(1);
				cannonballPLaceBody->SetRollingFriction(1);

				CollisionShape* shape = cannonNode->CreateComponent<CollisionShape>();
				shape->SetCapsule(2.0f, 6.0f);

				// и ядро в ней
				Node* cannonballNode = scene_->CreateChild("Cannonball");
				const Vector3 pos = cannonNode->GetChild("Forward", true)->GetChild("CannonballPlace", true)->GetWorldPosition();
				cannonballNode->SetPosition(pos);
				cannonballNode->SetScale(2.0f);

				StaticModel* cannonballObject = cannonballNode->CreateComponent<StaticModel>();
				cannonballObject->SetModel(cache->GetResource<Model>("Models/Cannonball.mdl"));
				cannonballObject->SetMaterial(cache->GetResource<Material>("Materials/Cannonball.xml"));
				cannonballObject->SetCastShadows(true);
				cannonballObject->SetOccluder(false);

				RigidBody* cannonballBody = cannonballNode->CreateComponent<RigidBody>();
				cannonballBody->SetCollisionLayer(1.0f);
				cannonballBody->SetMass(2.0f);
				cannonballBody->SetFriction(1);
				cannonballBody->SetRollingFriction(1.0);

				CollisionShape* cannonballShape = cannonballNode->CreateComponent<CollisionShape>();
				cannonballShape->SetSphere(0.8f);

				Shooting* shoot = cannonballNode->CreateComponent<Shooting>();
				shoot->SetParameters(pos, Vector3::RIGHT);

				isEmpty = false;
			}
			else if (i != 0 && z_OrientedWallCount > 0 && isEmpty == true)
			{
				//создаём блок
				
				Node* boxNode = scene_->CreateChild("Box");
				boxNode->SetPosition(Vector3(0, 2.5f, -boxScale * i));
				boxNode->SetScale(boxScale);
				StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
				boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
				boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
				boxObject->SetCastShadows(true);
				boxObject->SetOccluder(false);

				RigidBody* body = boxNode->CreateComponent<RigidBody>();
				body->SetCollisionLayer(1);
				body->SetMass(boxScale * 100.0f);
				CollisionShape* shape = boxNode->CreateComponent<CollisionShape>();
				shape->SetBox(Vector3::ONE);

				isEmpty = false;
				
			}

			/*
			если имеется коридор
			более трёх блоков, помещаем в него ниндзю
			*/
			if (z_OrientedWallCount > 1 && x_OrientedWallCount > 3 && hasNinja)
			{
				const float MODEL_MOVE_SPEED = 2.0f;

				Node* ninjaNode = scene_->CreateChild("Ninja");
				ninjaNode->SetPosition(Vector3(boxScale * j + boxScale - 2.7f, 2.5f, -boxScale * i));
				ninjaNode->SetRotation(Quaternion(0.0f, 90.0f, 0.0f));

				AnimatedModel* ninjaObject = ninjaNode->CreateComponent<AnimatedModel>();
				ninjaObject->SetModel(cache->GetResource<Model>("Models/NinjaSnowWar/Ninja.mdl"));
				ninjaObject->SetMaterial(cache->GetResource<Material>("Materials/NinjaSnowWar/Ninja.xml"));
				ninjaObject->SetCastShadows(true);

				Animation* walkAnimation = cache->GetResource<Animation>("Models/NinjaSnowWar/Ninja_Walk.ani");

				AnimationState* state = ninjaObject->AddAnimationState(walkAnimation);
				if (state)
				{
				state->SetWeight(1.0f);
				state->SetLooped(true);
				state->SetTime(Random(walkAnimation->GetLength()));
				}

				RigidBody* ninjaBody = ninjaNode->CreateComponent<RigidBody>();
				ninjaBody->SetCollisionLayer(1.0f);
				ninjaBody->SetMass(1.0f);
				ninjaBody->SetFriction(1);
				ninjaBody->SetRollingFriction(1.0);

				CollisionShape* ninjaShape = ninjaNode->CreateComponent<CollisionShape>();
				ninjaShape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));

				Mover* mover = ninjaNode->CreateComponent<Mover>();
				mover->SetParameters(MODEL_MOVE_SPEED, Vector3::FORWARD);

				hasNinja = false;
			}
		}
	}
	//конец создания лабиринта


	// Создаём красный дым
	Node* smokeNode = scene_->CreateChild("Smoke");
	smokeNode->SetPosition(Vector3(boxScale * 23, 2.5f, -boxScale * 21));

	BillboardSet* billboardObject = smokeNode->CreateComponent<BillboardSet>();
	billboardObject->SetNumBillboards(1);
	billboardObject->SetMaterial(cache->GetResource<Material>("Materials/Smoke.xml"));

	Billboard* bb = billboardObject->GetBillboard(0);
	bb->position_ = Vector3(Random(12.0f) - 6.0f, Random(8.0f) - 4.0f, Random(12.0f) - 6.0f);
	bb->size_ = Vector2(Random(2.0f) + 3.0f, Random(2.0f) + 3.0f);
	bb->rotation_ = Random() * 360.0f;
	bb->color_ = Color(256.0f, 0.0f, 0.0f);
	bb->enabled_ = true;

	// After modifying the billboards, they need to be "committed" so that the BillboardSet updates its internals
	billboardObject->Commit();
}

void CharacterDemo::CreateCharacter()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* objectNode = scene_->CreateChild("Jack");
    objectNode->SetPosition(Vector3(4.0f, 1.0f, -1.0f));

    // spin node
    Node* adjustNode = objectNode->CreateChild("AdjNode");
	adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));
    
    // Create the rendering component + animation controller
    AnimatedModel* object = adjustNode->CreateComponent<AnimatedModel>();
    object->SetModel(cache->GetResource<Model>("Models/Mutant/Mutant.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Models/Mutant/Materials/mutant_M.xml"));
    object->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    // Set the head bone for manual control
    object->GetSkeleton().GetBone("Mutant:Head")->animated_ = false;

    // Create rigidbody, and set non-zero mass so that the body becomes dynamic
    RigidBody* body = objectNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(1);
    body->SetMass(1.0f);

    // Устанавливаем нулевой угловой коэффициент, чтобы физика не поворачивала персонажа сама по себе.
	// Вместо этого мы будем управлять персонажем вручную
    body->SetAngularFactor(Vector3::ZERO);

    // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
    body->SetCollisionEventMode(COLLISION_ALWAYS);

    // Set a capsule shape for collision
    CollisionShape* shape = objectNode->CreateComponent<CollisionShape>();
    shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));

    // Create the character logic component, which takes care of steering the rigidbody
    // Remember it so that we can set the controls. Use a WeakPtr because the scene hierarchy already owns it
    // and keeps it alive as long as it's not removed from the hierarchy
    character_ = objectNode->CreateComponent<Character>();
}

void CharacterDemo::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText(
        L"Управление с помощью WASD и мыши\n"
        L"пробел - прыжок, F - менять камеру"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}

void CharacterDemo::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    Input* input = GetSubsystem<Input>();

    if (character_)
    {
        // Clear previous controls
        character_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using touch utility class
        if (touch_)
            touch_->UpdateTouches(character_->controls_);

        // Update controls using keys
        UI* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            if (!touch_)
            {
                character_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
                character_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
                character_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
                character_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            }
            character_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

            // Add character yaw & pitch from the mouse motion or touch input
            if (touchEnabled_)
            {
                for (unsigned i = 0; i < input->GetNumTouches(); ++i)
                {
                    TouchState* state = input->GetTouch(i);
                    if (!state->touchedElement_)    // Touch on empty space
                    {
                        Camera* camera = cameraNode_->GetComponent<Camera>();
                        if (!camera)
                            return;

                        Graphics* graphics = GetSubsystem<Graphics>();
                        character_->controls_.yaw_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.x_;
                        character_->controls_.pitch_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics->GetHeight() * state->delta_.y_;
                    }
                }
            }
            else
            {
                character_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                character_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }
            // Limit pitch
            character_->controls_.pitch_ = Clamp(character_->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
            character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));

            // Переключение между камерами
            if (input->GetKeyPress(KEY_F))
                viewFromAbove_ = !viewFromAbove_;
        }
    }
}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!character_)
        return;

    Node* characterNode = character_->GetNode();

    // Get camera lookat dir from character yaw + pitch
    Quaternion rot = characterNode->GetRotation();
    Quaternion dir = rot * Quaternion(character_->controls_.pitch_, Vector3::RIGHT);

    // Turn head to camera pitch, but limit to avoid unnatural animation
    Node* headNode = characterNode->GetChild("Mutant:Head", true);
    float limitPitch = Clamp(character_->controls_.pitch_, -45.0f, 45.0f);
    Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
    headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));

    if (viewFromAbove_)
    {
        cameraNode_->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 20.15f, -20.15f));
		cameraNode_->SetRotation(dir * Quaternion(10.0f, 0.0f, 0.0f));
    }
    else
    {
        // Third person camera: position behind the character
        Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.7f, 0.0f);

        // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = touch_ ? touch_->cameraDistance_ : CAMERA_INITIAL_DIST;
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
        if (result.body_)
            rayDistance = Min(rayDistance, result.distance_);
        rayDistance = Clamp(rayDistance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);

        cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
        cameraNode_->SetRotation(dir);
    }
}
