#define _CRT_SECURE_NO_WARNINGS
#define THATBOY_DEBUG 0

#include <easyx.h>
#include<ctime>
#include <stack>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

enum MazeValue : std::uint8_t
{
	Space,
	Wall
};
constexpr size_t M = 41;

std::vector<std::vector<MazeValue>> Map;

void CreateMaze(std::vector<std::vector<MazeValue>>&, size_t);		// 创建地图
void DisplayMaze(const std::vector<std::vector<MazeValue>>&, size_t);// 展示地图Maze

int main()
{
	int bsize = 4;
	CreateMaze(Map, M);
	initgraph(M * bsize, M * bsize);

	DisplayMaze(Map, bsize);

	using namespace std;
	ofstream of;
	of.open("map.txt");

	of << M << ' ' << M << endl;
	for (size_t i = 0; i < M; i++)
	{
		for (size_t j = 0; j < M; j++)
		{
			of << (Map[i][j] == Space ? 0 : 1) << ' ';
		}
		of << endl;
	}
	of.close();
	saveimage(TEXT("map.bmp"));

	Sleep(8000);
	closegraph();
	return 0;
}

// 速度很慢，不用于迷宫生成
bool TestConnect(std::vector<std::vector<MazeValue>>& map, size_t size, int x1, int y1, int x2, int y2)
{
	// 存储点
	struct Point
	{
		int x;
		int y;
		Point(int xx, int yy)
			: x(xx)
			, y(yy)
		{}
		Point(const Point&) = default;
	};
	std::vector<std::vector<bool>> book(size, std::vector<bool>(size, false));	// 通路标记
	std::stack<Point> PtStack;													// 点栈

	PtStack.push(Point(x1, y1));

	while (!PtStack.empty())
	{
		auto pt(PtStack.top());
		PtStack.pop();

		if (pt.x < 0 || pt.x >= (int)size || pt.y < 0 || pt.y >= (int)size || book[pt.x][pt.y])
			continue;

		book[pt.x][pt.y] = true;
		if (pt.x == x2 && pt.y == y2)
			return true;
		else if (map[pt.x][pt.y] == Wall)
			continue;

		PtStack.push(Point(pt.x - 1, pt.y));
		PtStack.push(Point(pt.x + 1, pt.y));
		PtStack.push(Point(pt.x, pt.y - 1));
		PtStack.push(Point(pt.x, pt.y + 1));
	}
	return false;

};

void CreateMaze(std::vector<std::vector<MazeValue>>& map, size_t size)
{
	srand((size_t)time(0));

	// 存储墙信息类
	struct WallInfo
	{
		WallInfo(const WallInfo& w)
			: x(w.x)
			, y(w.y)
			, bHor(w.bHor)
		{}
		WallInfo(int xx, int yy, int b)
			: x(xx)
			, y(yy)
			, bHor(b)
		{}
		WallInfo& operator =(const WallInfo& w)
		{
			x = w.x;
			y = w.y;
			bHor = w.bHor;
			return *this;
		}
		int x;
		int y;
		bool bHor;// 是否水平

	private:
		int rank = rand();
	public:
		bool operator <(const WallInfo& w)const
		{
			return rank < w.rank;
		}
	};
	// 存储点
	struct Point
	{
		int x;
		int y;
		Point(int xx, int yy)
			: x(xx)
			, y(yy)
		{}
		Point(const Point&) = default;
	};


	std::vector<std::vector<bool>> book(size, std::vector<bool>(size, false));	// 通路标记
	std::stack<Point> PtStack;													// 点栈
	std::vector<WallInfo> WallList;
	WallList.reserve((size - 1) * ((size - 1) / 2 - 1));
	// 初始化
	map.clear();
	for (size_t i = 0; i < size; i++)
		map.push_back(std::vector<MazeValue>(size, Wall));

	// 挖洞
	for (size_t i = 0; i < size / 2; i++)
	{
		for (size_t j = 0; j < size / 2; j++)
		{
			map[i * 2 + 1][j * 2 + 1] = Space;	// 打通
		}
	}
	// 存储墙	
	for (size_t i = 1; i < size - 1; i++)
	{
		if (i % 2 == 0)
			WallList.push_back(WallInfo(i, 1, false));
		for (size_t j = 1; j < size / 2; j++)
			WallList.push_back(WallInfo(i, j * 2 + (i % 2 ? 0 : 1), i % 2));
	}

	// 排序
	std::sort(WallList.begin(), WallList.end());

	// 用于测试连通性
	auto TestConnect = [&](int x1, int y1, int x2, int y2)
	{
		while (!PtStack.empty())
			PtStack.pop();
		for (size_t i = 0; i < size; i++)
		{
			for (size_t j = 0; j < size; j++)
			{
				book[i][j] = false;
			}
		}

		PtStack.push(Point(x1, y1));

		while (!PtStack.empty())
		{
			auto pt(PtStack.top());
			PtStack.pop();

			if (pt.x < 0 || pt.x >= (int)size || pt.y < 0 || pt.y >= (int)size || book[pt.x][pt.y])
				continue;

			book[pt.x][pt.y] = true;
			if (pt.x == x2 && pt.y == y2)
				return true;
			else if (map[pt.x][pt.y] == Wall)
				continue;

			PtStack.push(Point(pt.x - 1, pt.y));
			PtStack.push(Point(pt.x + 1, pt.y));
			PtStack.push(Point(pt.x, pt.y - 1));
			PtStack.push(Point(pt.x, pt.y + 1));
		}
		return false;

	};

	// 凿壁
	for (auto& wall : WallList)
		if (!(wall.bHor ? TestConnect(wall.x, wall.y - 1, wall.x, wall.y + 1) : TestConnect(wall.x - 1, wall.y, wall.x + 1, wall.y)))
			map[wall.x][wall.y] = Space;	// 打通
	WallList.clear();
}

void DisplayMaze(const std::vector<std::vector<MazeValue>>& map, size_t block_size)
{
	for (size_t i = 0; i < map.size(); i++)
	{
		for (size_t j = 0; j < map.front().size(); j++)
		{
			setfillcolor(map[i][j] == Space ? WHITE : BLACK);
			solidrectangle(i * block_size, j * block_size, i * block_size + block_size, j * block_size + block_size);
		}
	}
}
