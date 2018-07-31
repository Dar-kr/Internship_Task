//============================================================================
// Name        : shortest_path.cpp
// Author      : Daryna Kravets
// Description : Materialise Internship Task.
//				 Shortest path between given points over triangle surface.
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
using namespace std;
const float PI = 3.1415926F;

//classes for triangle facets
class point
{
public:
	float x;
	float y;
	float z;
};
class facet
{
public:
	point p[3];
};

//check equality of two points
bool equal(point p1, point p2)
{
	bool res = false;
	if (p1.x == p2.x)
		if (p1.y == p2.y)
			if (p1.z == p2.z)
				res = true;
	return res;
}

//return 0 if STL file is binary, 1 if file is ASCII, -1 if error
int filecheck(string filename)
{
	int res;
	ifstream file(filename);
	if (!file.is_open())
		res = -1;
	else
	{
		char* fivebyte = new char[5];
		file.read(fivebyte, 5);
		string key = "solid";
		res = 1;
		for (int i = 0; i < 5; i++)
			if (fivebyte[i] != key[i])
			{
				res = 0;
				break;
			}
		delete fivebyte;
	}
	file.close();
	return res;
}

//read .stl file. Return number of facets if ok, 0 if error.
unsigned long fileread(string filename, vector<facet>& surface)
{
	unsigned long n = 0;
	int ftype = filecheck(filename);
	if (ftype >= 0)
	{
		if (ftype == 0) //binary file
		{
			ifstream file(filename, ios::in | ios::binary);
			facet tr;
			char buffer[4];
			file.seekg(80);
			file.read(buffer, 4);
			n = *((unsigned long*)buffer);
			for (unsigned long i = 0; i < n; i++)
			{
				file.seekg(12, ios_base::cur);
				for (short j = 0; j < 3; j++)
				{
					file.read(buffer, 4);
					tr.p[j].x = *((float*)buffer);
					file.read(buffer, 4);
					tr.p[j].y = *((float*)buffer);
					file.read(buffer, 4);
					tr.p[j].z = *((float*)buffer);
				}
				surface.push_back(tr);
				file.seekg(2, ios_base::cur);
			}
			file.close();
		}
		else //ASCII file
		{
			ifstream file(filename);
			facet tr;
			char buffer[256];
			char key[7] = "vertex";
			file.getline(buffer, 256);
			short int i = 0;
			while (file >> buffer)
			{
				bool res = true;
				for (short int j = 0; j < 6; j++)
					if (buffer[j] != key[j])
					{
						res = false;
						break;
					}
				if (res)
				{
					file >> tr.p[i].x;
					file >> tr.p[i].y;
					file >> tr.p[i].z;
					i++;
					if (i == 3)
					{
						i = 0;
						n++;
						surface.push_back(tr);
					}
				}
			}
			file.close();
		}
	}
	else
		cout << "Can't read file";
	return n;
}

//found a point of intersection of a line and a plane
point line_plane(point line1, point line2, facet plane)
{
	//line: x(y2l-y1l)-y(x2l-x1l)=x1l*y2l-y1l*x2l; x(z2l-z1l)-z(x2l-x1l)=x1l*z2l-z1l*x2l; y(z2l-z1l)-z(y2l-y1l)=y1l*z2l-z1l*y2l. (2 from 3)
	//plane: A1*x+A2*y+A3*z=A1*x1+A2*y1+A3*z1,
	//A1=(y2-y1)(z3-z1)-(y3-y1)(z2-z1); A2=(z2-z1)(x3-x1)-(z3-z1)(x2-x1); A3=(x2-x1)(y3-y1)-(x3-x1)(y2-y1).
	//Gauss' method.
	point res;
	res.x = numeric_limits<float>::max();
	res.y = numeric_limits<float>::max();
	res.z = numeric_limits<float>::max();
	float mt[4][4];
	int rows[4] = { 0, 0, 0, 0 };
	mt[0][0] = (plane.p[1].y - plane.p[0].y) * (plane.p[2].z - plane.p[0].z) - (plane.p[2].y - plane.p[0].y) * (plane.p[1].z - plane.p[0].z);
	mt[0][1] = (plane.p[1].z - plane.p[0].z) * (plane.p[2].x - plane.p[0].x) - (plane.p[2].z - plane.p[0].z) * (plane.p[1].x - plane.p[0].x);
	mt[0][2] = (plane.p[1].x - plane.p[0].x) * (plane.p[2].y - plane.p[0].y) - (plane.p[2].x - plane.p[0].x) * (plane.p[1].y - plane.p[0].y);
	mt[0][3] = mt[0][0] * plane.p[0].x + mt[0][1] * plane.p[0].y + mt[0][2] * plane.p[0].z;
	mt[1][0] = line2.y - line1.y;
	mt[1][1] = line1.x - line2.x;
	mt[1][2] = 0;
	mt[1][3] = line1.x * line2.y - line1.y * line2.x;
	mt[2][0] = line2.z - line1.z;
	mt[2][1] = 0;
	mt[2][2] = line1.x - line2.x;
	mt[2][3] = line1.x * line2.z - line1.z * line2.x;
	mt[3][0] = 0;
	mt[3][1] = line2.z - line1.z;
	mt[3][2] = line1.y - line2.y;
	mt[3][3] = line1.y * line2.z - line1.z * line2.y;
	for (int i = 0; i < 3; i++)
	{
		int k = 4;
		for (int ii = 0; ii < 4; ii++)
		{
			if ((mt[ii][i] != 0) && (rows[ii] == 0))
			{
				for (int j = 3; j >= i; j--)
				{
					mt[ii][j] /= mt[ii][i];
					k = ii;
				}
			}
		}
		if (k < 4)
		{
			rows[k] = i + 1;
			for (int j = 3; j >= i; j--)
				for (int row = 0; row < 4; row++)
					if (row != k) //&& (mt[row][i]))
						mt[row][j] -= mt[k][j] * mt[row][i];
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (rows[i] == 0)
		{
			if (mt[i][3] == 0)
			{
				res = line1;
				break;
			}
			else
				break;
		}
		else if (rows[i] == 1)
			res.x = mt[i][3];
		else if (rows[i] == 2)
			res.y = mt[i][3];
		else if (rows[i] == 3)
			res.z = mt[i][3];
	}
	return res;
}

//return a length of line between two points
float length(point p1, point p2)
{
	float ln = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
	return ln;
}

//check point position. If i_point lie between points, return true, else false.
bool pointcheck(point i_point, point line1, point line2)
{
	bool flag = false;
	float so_small = length(line1, line2) / 100000.0F;
	float dlt = length(i_point, line1) + length(i_point, line2) - length(line1, line2);
	if (dlt <= so_small)
		flag = true;
	return flag;
}

//return a point in plane that makes angle with start plane
point third_point(point p1, point p2, point ps, float angle)
{
	point res;
	point v; 
	float b = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) + (p2.z - p1.z)*(p2.z - p1.z));
	v.x = (p2.x - p1.x) / b;
	v.y = (p2.y - p1.y) / b;
	v.z = (p2.z - p1.z) / b;
	float cs = cos(angle);
	float sn = sin(angle);
	float M_angle[3][3];
	M_angle[0][0] = cs + (1 - cs) * v.x * v.x;
	M_angle[1][0] = (1 - cs) * v.y * v.x + sn * v.z;
	M_angle[2][0] = (1 - cs) * v.z * v.x + sn * v.y;
	M_angle[0][1] = (1 - cs) * v.x * v.y + sn * v.z;
	M_angle[1][1] = cs + (1 - cs) * v.y * v.y;
	M_angle[2][1] = (1 - cs) * v.z * v.y + sn * v.x;
	M_angle[0][2] = (1 - cs) * v.x * v.z + sn * v.y;
	M_angle[1][2] = (1 - cs) * ps.y * ps.z + sn * ps.x;
	M_angle[2][2] = cs + (1 - cs) * v.z * v.z;
	res.x = M_angle[0][0] * ps.x + M_angle[0][1] * ps.y + M_angle[0][2] * ps.z;
	res.y = M_angle[1][0] * ps.x + M_angle[1][1] * ps.y + M_angle[1][2] * ps.z;
	res.z = M_angle[2][0] * ps.x + M_angle[2][1] * ps.y + M_angle[2][2] * ps.z;
	return res;
}

//choose a start point for round move. (p1 != p2)!!! Line start-p1 is ortogonal to p1-p2. 
point start_point(point p1, point p2)
{
	point res;
	res = p1;
	float dlt[3];
	dlt[0] = p2.x - p1.x;
	dlt[1] = p2.y - p1.y;
	dlt[2] = p2.z - p1.z;
	if (dlt[0] == 0)
	{
		if (dlt[1] == 0)
			res.x += dlt[2];
		else res.x += dlt[1];
	}
	else 
	{
		if (dlt[1] == 0)
			res.y += dlt[0];
		else
			if (dlt[2] == 0)
				res.z += dlt[0];
			else
			{
				res.x = (p1.x + p2.x) / 2;
				res.y = (p1.y + p2.y) / 2;
				res.z = (2 * p1.z * dlt[2] - dlt[0] * dlt[0] - dlt[1] * dlt[1]) / (2 * dlt[2]);
			}
	}
	return res;
}

//evaluate the path for given plane
float intersect(facet plane, vector<facet>& surface, vector<point>& path1, vector<point>& path2)
{
	float path_length[2] = { 0.0, 0.0 };
	int path_numb = 0;
	path1.clear();
	path2.clear();
	for (facet tr : surface)
	{
		int k = 3;
		for (int m = 0; m < 3; m++)
			if (equal(plane.p[0], tr.p[m]))
				k = m;
		if (k < 3)
		{
			point p1, p2, p_other;
			if (k == 0)
			{
				p_other = tr.p[0];
				p1 = tr.p[1];
				p2 = tr.p[2];
			}
			else if (k == 1)
			{
				p1 = tr.p[0];
				p_other = tr.p[1];
				p2 = tr.p[2];
			}
			else
			{
				p1 = tr.p[0];
				p2 = tr.p[1];
				p_other = tr.p[2];
			}

			int k1 = 3;
			for (int m = 0; m < 3; m++)
				if ((pointcheck(plane.p[1], p1, p_other)) || (pointcheck(plane.p[1], p2, p_other)))
					k1 = m;
			if (k1 < 3)
			{
				if (path_numb == 0)
				{
					path1.push_back(plane.p[0]);
					path1.push_back(plane.p[1]);
				}
				if (path_numb == 1)
				{
					path2.push_back(plane.p[0]);
					path2.push_back(plane.p[1]);
				}
				path_length[path_numb] = length(plane.p[0], plane.p[1]);
				break;
			}

			point cut = line_plane(p1, p2, plane);
			if (pointcheck(cut, p1, p2))
			{
				if (path_numb == 0)
					path1.push_back(plane.p[0]);
				if (path_numb == 1)
					path2.push_back(plane.p[0]);
				bool full = false;
				point prev = plane.p[0];
				while (!full)
				{
					if ((pointcheck(plane.p[0], cut, prev)) && (path_length[path_numb] != 0))
					{
						path_length[path_numb] = -1;
						full = true;
					}
					else if (pointcheck(plane.p[1], cut, prev))
					{
						path_length[path_numb] += length(plane.p[1], prev);
						if (path_numb == 0)
							path1.push_back(plane.p[1]);
						if (path_numb == 1)
							path2.push_back(plane.p[1]);
						full = true;
					}
					else
					{
						path_length[path_numb] += length(cut, prev);
						if (!equal(cut, prev))
						{
							if (path_numb == 0)
								path1.push_back(cut);
							if (path_numb == 1)
								path2.push_back(cut);
						}
						prev = cut;
						for (facet next : surface)
						{
							int t1 = 3;
							int t2 = 3;
							for (int ii = 0; ii < 3; ii++)
								if (equal(p1, next.p[ii]))
									t1 = ii;
							for (int ii = 0; ii < 3; ii++)
								if (equal(p2, next.p[ii]))
									t2 = ii;
							if ((t1 < 3) && (t2 < 3))
							{
								if (!equal(p_other, next.p[3 - t1 - t2]))
								{
									cut = line_plane(p1, next.p[3 - t1 - t2], plane);
									if (!pointcheck(cut, p1, next.p[3 - t1 - t2]))
									{
										cut = line_plane(p2, next.p[3 - t1 - t2], plane);
										if (pointcheck(cut, p2, next.p[3 - t1 - t2]))
										{
											p_other = p1;
											p1 = next.p[3 - t1 - t2];
											break;
										}
									}
									else
									{
										p_other = p2;
										p2 = p1;
										p1 = next.p[3 - t1 - t2];
										break;
									}
								}
							}
						}

					}
				}
				path_numb++;
			}
		}

		if (path_numb == 2)
			break;
	}
	if (path_length[1] != 0)
	{
		if (path_length[0] <= path_length[1])
			path2.clear();
		else
		{
			path1.clear();
			path_length[0] = path_length[1];
		}
	}
	else path2.clear();
	return path_length[0];
}

int main()
{
	cout << "Enter filename:";
	string filename;
	cin >> filename;
	vector<facet> surface;
	vector<point> path1, path2, path_sm;
	float sm_path, cur_path;
	sm_path = numeric_limits<float>::max();
	unsigned long numb = fileread(filename, surface);
	facet pl;		
	cout << "Enter 6 floats - 3 for start point (on vertex of surface) and 3 for finish point (anywere):\n";
	cin >> pl.p[0].x >> pl.p[0].y >> pl.p[0].z;
	cin >> pl.p[1].x >> pl.p[1].y >> pl.p[1].z;
	point pst = start_point(pl.p[0], pl.p[1]);
	for (unsigned long i = 0; i < numb * numb; i++)
	{
		float angle = PI * (float)i / (float)(numb * numb);
		pl.p[2] = third_point(pl.p[0], pl.p[1], pst, angle);
		cur_path = intersect(pl, surface, path1, path2);
		if ((cur_path < sm_path) && (cur_path > 0))
		{
			sm_path = cur_path;
			if (!path1.empty())
				path_sm = *(&(path1));
			else path_sm = *(&(path2));
		}
	}
	if (sm_path == numeric_limits<float>::max())
		cout << "no path\n";
	else
	{
		cout << sm_path << "\nPoints of path:\n";
		for (point p_path : path_sm)
			cout << p_path.x << ' ' << p_path.y << ' ' << p_path.z << '\n';
	}	
	return 0;
}
