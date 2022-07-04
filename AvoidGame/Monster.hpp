class Monster
{
public:
	int x, y;
	Monster()
	{
		x = 40;
		y = 7;
	}
	~Monster()
	{

	}
	void Moving()
	{
		if (x <= 0) x = 40;
		x--;
	}
};