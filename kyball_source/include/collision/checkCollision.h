inline virtual CollisionParams checkCollision(CollisionObject* object, int TTL = 2) {
	if (TTL == 0) return CollisionParams::NO_COLLISION;
	return object->checkCollision(this, --TTL).swap(); /// <swap> = return correct order!!!
};

inline virtual CollisionParams checkCollision(Sphere* sphere, int TTL) {
	return CollisionExecuter::checkCollision(sphere, this).swap(); /// <swap> = return correct order!!!
};
