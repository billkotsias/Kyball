virtual double distance(const CollisionObject* object, int TTL = 2)  const {
	if (TTL == 0) return Infinite;
	return object->distance(this, --TTL);
};

virtual double distance(const Sphere* sphere, int TTL)  const {
	return CollisionExecuter::distance(sphere, this);
};
