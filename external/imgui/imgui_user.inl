

bool operator ==(ImVec2 v1, ImVec2 v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

bool operator !=(ImVec2 v1, ImVec2 v2)
{
	return !(v1 == v2);
}
