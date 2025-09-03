// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Welker Gergo
// Neptun : ECKCA4
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char *const vertexSource = R"(
#version 330			   // Shader 3.3
	precision highp float; // normal floats, makes no difference on desktop computers
 
uniform mat4 MVP;				 // uniform variable, the Model-View-Projection transformation matrix
layout(location = 0) in vec2 vp; // Varying input: vp = vertex position is expected in attrib array 0
 
void main()
{
	gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP; // transform vp from modeling space to normalized device space
}
)";

// fragment shader in GLSL
const char *const fragmentSource = R"(
#version 330				// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
 
uniform vec3 color; // uniform variable, the color of the primitive
out vec4 outColor;	// computed color of the current pixel
 
void main()
{
	outColor = vec4(color, 1); // computed color is the color of the primitive
}
)";

#define CUSTOM_PI 3.14159265358979323846264338327950288f

GPUProgram gpuProgram;
unsigned int vao;
bool simulationStarted = false;

class HyperbolicField
{
public:
	static vec3 DeviceToHyperbolicPosition(float cX, float cY)
	{
		CircleClamp(cX, cY, sqrt(2.0f) / 1.5f);
		return vec3(cX, cY, 1.0f) / sqrt(1.0f - cX * cX - cY * cY);
	}
	static void CircleClamp(float &v1, float &v2, float maxLength)
	{
		float length = sqrt(v1 * v1 + v2 * v2);
		if (length >= maxLength)
		{
			float div = length / maxLength;
			v1 /= div;
			v2 /= div;
		}
	}
	static float Distance(vec3 p1, vec3 p2)
	{
		float dis = acoshf((-1.0f) * DotProduct(p1, p2));
		return dis > 10e-7f ? dis : 10e-7f;
	}
	static vec3 LineSegment(vec3 p, vec3 m, float projectDistance)
	{
		float distance = Distance(p, m);
		vec3 v = (m - (p * (coshf(distance)))) / sinhf(distance);
		return p * coshf(projectDistance) + v * sinhf(projectDistance);
	}
	static vec3 TangentLine(vec3 p, vec3 m)
	{
		float distance = Distance(p, m);
		vec3 v = (m - (p * (coshf(distance)))) / sinhf(distance);
		return normalize(v);
	}
	static vec3 ShiftPoint(vec3 start, vec3 end, vec3 p)
	{
		vec3 oMP = LineSegment(start, end, Distance(start, end) * 0.25f);
		vec3 eMP = LineSegment(start, end, Distance(start, end) * 0.75f);
		vec3 oM = LineSegment(p, oMP, Distance(p, oMP) * 2.0f);
		vec3 eM = LineSegment(oM, eMP, Distance(oM, eMP) * 2.0f);

		return eM;
	}
	static vec3 ProjectPoint(vec3 vec)
	{
		return vec3(vec.x / vec.z, vec.y / vec.z, 1.0f);
	}
	static float DotProduct(vec3 v1, vec3 v2)
	{
		return (v1.x * v2.x + v1.y * v2.y - v1.z * v2.z);
	}
};

class GraphPoint
{

	vec3 graphPointCenter;
	std::vector<vec3> circleVertices;
	std::vector<GraphPoint *> connections;
	vec3 color;
	vec3 force;

public:
	GraphPoint(int circleDivision, float circleRadius)
	{
		float r = 2.0f * (float)random() / (RAND_MAX);
		float angle = random() * 2.0f * CUSTOM_PI;
		float x = r * cosf(angle);
		float y = r * sinf(angle);
		float w = sqrt((x * x) + (y * y) + 1.0f);
		graphPointCenter = vec3(x, y, w);

		for (int i = 0; i < circleDivision; i++)
		{
			float angle = 2.0f * CUSTOM_PI * (float)i / (float)circleDivision;
			float cvX = circleRadius * cosf(angle);
			float cvY = circleRadius * sinf(angle);
			float cvW = sqrt((cvX * cvX) + (cvY * cvY) + 1.0f);
			circleVertices.push_back(HyperbolicField::ShiftPoint(vec3(0, 0, 1), graphPointCenter, vec3(cvX, cvY, cvW)));
		}

		color = vec3((float)rand() / (RAND_MAX) + 0.1f, (float)rand() / (RAND_MAX) + 0.1f, (float)rand() / (RAND_MAX) + 0.1f);
	}

	void PushPoint(vec3 start, vec3 end)
	{
		start = vec3(start.x, start.y, sqrt((start.x * start.x) + (start.y * start.y) + 1.0f));
		end = vec3(end.x, end.y, sqrt((end.x * end.x) + (end.y * end.y) + 1.0f));
		graphPointCenter = HyperbolicField::ShiftPoint(start, end, graphPointCenter);
		for (size_t i = 0; i < circleVertices.size(); i++)
		{
			circleVertices[i] = HyperbolicField::ShiftPoint(start, end, circleVertices[i]);
		}
	}

	vec3 &GetForce()
	{
		return force;
	}
	void SetForce(vec3 f)
	{
		force = f;
	}
	vec3 &GetPosition()
	{
		return graphPointCenter;
	}
	std::vector<vec3> &GetCircleVertices()
	{
		return circleVertices;
	}
	std::vector<GraphPoint *> &GetConnections()
	{
		return connections;
	}
	void AddConnection(GraphPoint *gp)
	{
		connections.push_back(gp);
	}
	bool ContainsConnection(GraphPoint *gp)
	{
		for (size_t i = 0; i < connections.size(); i++)
		{
			if (gp == connections[i])
			{
				return true;
			}
		}
		return false;
	}
	vec3 &GetCircleColor()
	{
		return color;
	}
};

class HyperbolicGraph
{

	std::vector<GraphPoint> graphPoints;
	vec3 mouseClickStartPos;
	vec3 mouseDragPos;
	std::vector<GraphPoint> initialState;
	int GetRandomGraphPointIndex(int excludeIndex)
	{
		int randIndex = rand() % graphPoints.size();
		if (randIndex == excludeIndex)
		{
			return GetRandomGraphPointIndex(excludeIndex);
		}
		else
		{
			return randIndex;
		}
	}

public:
	HyperbolicGraph(int graphPointsCount, int graphCircleDivision, float graphCircleRadius, float edgeFillPercent)
	{
		srand(0);
		for (size_t i = 0; i < graphPointsCount; i++)
		{
			graphPoints.push_back(GraphPoint(graphCircleDivision, graphCircleRadius));
		}
		edgeFillPercent = abs(edgeFillPercent) > 100.0f ? 100.0f : abs(edgeFillPercent);
		int edgeCount = (int)((((float)graphPointsCount * ((float)graphPointsCount - 1.0f)) / 2.0f) * (edgeFillPercent / 100.0f));
		int addedEdges = 0;
		while (addedEdges < edgeCount)
		{
			int randomStart = rand() % graphPoints.size();
			int randomEnd = GetRandomGraphPointIndex(randomStart);
			if (!graphPoints[randomStart].ContainsConnection(&graphPoints[randomEnd]) && !graphPoints[randomEnd].ContainsConnection(&graphPoints[randomStart]))
			{
				graphPoints[randomStart].AddConnection(&graphPoints[randomEnd]);
				graphPoints[randomEnd].AddConnection(&graphPoints[randomStart]);

				++addedEdges;
			}
		}
		for (size_t i = 0; i < graphPoints.size(); i++)
		{
			initialState.push_back(GraphPoint(graphPoints[i]));
		}

		printf("Graph setup with %d points, %d circle divisions, %.4f circle radius and %d edges (%.1f%%) \n", graphPointsCount, graphCircleDivision, graphCircleRadius, edgeCount, edgeFillPercent);
	}
	void MoveGraph()
	{
		for (size_t i = 0; i < graphPoints.size(); i++)
		{
			graphPoints[i].PushPoint(mouseClickStartPos, mouseDragPos);
		}
		mouseClickStartPos = mouseDragPos;
		glutPostRedisplay();
	}
	void DisplayGraph()
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		int colorLocation = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f);

		std::vector<vec3> edges;
		std::vector<GraphPoint *> addedPoints;
		for (size_t i = 0; i < graphPoints.size(); i++)
		{
			vec3 center = HyperbolicField::ProjectPoint(graphPoints[i].GetPosition());
			std::vector<GraphPoint *> connections = graphPoints[i].GetConnections();

			for (size_t j = 0; j < connections.size(); j++)
			{
				bool edgeAdded = false;
				for (size_t k = 0; k < addedPoints.size(); k++)
				{
					if (addedPoints[k] == connections[j])
					{
						edgeAdded = true;
						break;
					}
				}
				if (!edgeAdded)
				{
					edges.push_back(center);
					edges.push_back(HyperbolicField::ProjectPoint(connections[j]->GetPosition()));
				}
			}
			addedPoints.push_back(&graphPoints[i]);
		}
		glBufferData(GL_ARRAY_BUFFER,
					 sizeof(vec3) * edges.size(),
					 &edges[0],
					 GL_DYNAMIC_DRAW);

		glDrawArrays(GL_LINES, 0, edges.size());

		glClear(GL_DEPTH_BUFFER_BIT);

		for (size_t i = 0; i < graphPoints.size(); i++)
		{
			vec3 color = graphPoints[i].GetCircleColor();
			glUniform3f(colorLocation, color.x, color.y, color.z);

			std::vector<vec3> circleVertices = graphPoints[i].GetCircleVertices();
			std::vector<vec3> projectedVertices;
			for (size_t j = 0; j < circleVertices.size(); j++)
			{
				vec3 pos = HyperbolicField::ProjectPoint(circleVertices[j]);
				projectedVertices.push_back(pos);
			}

			glBufferData(GL_ARRAY_BUFFER,
						 sizeof(vec3) * projectedVertices.size(),
						 &projectedVertices[0],
						 GL_DYNAMIC_DRAW);

			glDrawArrays(GL_TRIANGLE_FAN, 0, projectedVertices.size());
		}

		glutSwapBuffers();

	}
	void SimulationSetup()
	{

		for (size_t i = 0; i < graphPoints.size(); i++)
		{
			graphPoints[i] = GraphPoint(initialState[i]);
			graphPoints[i].PushPoint(graphPoints[i].GetPosition(), HyperbolicField::LineSegment(graphPoints[i].GetPosition(), vec3(0.0f, 0.0f, 1.0f), (float)random() / RAND_MAX * 2.0f * HyperbolicField::Distance(vec3(0.0f, 0.0f, 1.0f), graphPoints[i].GetPosition())));
		}
	}
	void Simulate()
	{
		float preferredDistance = 0.5f;
		for (size_t i = 0; i < 20; i++)
		{
			for (size_t i = 0; i < graphPoints.size(); i++)
			{
				graphPoints[i].SetForce(vec3(0.0f, 0.0f, 0.0f));
			}
			for (size_t i = 0; i < graphPoints.size(); i++)
			{
				for (size_t j = 0; j < graphPoints.size(); j++)
				{
					if (i != j)
					{
						float distance = HyperbolicField::Distance(graphPoints[i].GetPosition(), graphPoints[j].GetPosition());

						if (graphPoints[i].ContainsConnection(&graphPoints[j]))
						{
							float mul = (distance * 3.0f - preferredDistance) / distance * 2.0f;
							graphPoints[i].SetForce(graphPoints[i].GetForce() + mul * HyperbolicField::TangentLine(graphPoints[i].GetPosition(), graphPoints[j].GetPosition()));
						}
						else
						{
							float mul = (distance - preferredDistance * 2.0f) / distance;
							graphPoints[i].SetForce(graphPoints[i].GetForce() + mul * HyperbolicField::TangentLine(graphPoints[i].GetPosition(), graphPoints[j].GetPosition()));
						}
					}
				}
			}
			for (size_t i = 0; i < graphPoints.size(); i++)
			{
				vec3 diff = graphPoints[i].GetForce() * 0.005f;
				graphPoints[i].PushPoint(graphPoints[i].GetPosition(), graphPoints[i].GetPosition() + diff);
			}
		}
		glutPostRedisplay();
	}

	void SetMouseClickStartPos(vec3 pos)
	{
		mouseClickStartPos = pos;
	}
	vec3 GetMouseClickStartPos()
	{
		return mouseClickStartPos;
	}
	void SetMouseDragPos(vec3 pos)
	{
		mouseDragPos = pos;
	}
	vec3 &GetMouseDragPos()
	{
		return mouseDragPos;
	}
};

int graphPoints = 50;
HyperbolicGraph hyperbolicGraph(graphPoints, 20, 0.025f, 5.0f);

void onInitialization()
{
  printf("Press the Space Bar to setup the simulation\n");
	glViewport(0, 0, windowWidth, windowHeight);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,
						  3, GL_FLOAT, GL_FALSE,
						  0, NULL);

	gpuProgram.create(vertexSource, fragmentSource, "outColor");

	float MVPtransf[4][4] = {1, 0, 0, 0,
							 0, 1, 0, 0,
							 0, 0, 1, 0,
							 0, 0, 0, 1};

	int location = glGetUniformLocation(gpuProgram.getId(), "MVP");
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);
}

void onDisplay()
{
	hyperbolicGraph.DisplayGraph();
}

void onKeyboard(unsigned char key, int pX, int pY)
{
	if (key == 'd')
		glutPostRedisplay();

	if (key == 32) // space bar
	{
		hyperbolicGraph.SimulationSetup();
		simulationStarted = true;
	}
}

void onKeyboardUp(unsigned char key, int pX, int pY)
{
	printf("Keyboard %d at (%3.2f, %3.2f)\n", key, (float)pX, (float)pY);
}

void onMouseMotion(int pX, int pY)
{

	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;

	hyperbolicGraph.SetMouseDragPos(HyperbolicField::DeviceToHyperbolicPosition(cX, cY));
	hyperbolicGraph.MoveGraph();
}

void onMouse(int button, int state, int pX, int pY)
{
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;

	const char *buttonStat;
	switch (state)
	{
	case GLUT_DOWN:
		buttonStat = "pressed";
		break;
	case GLUT_UP:
		buttonStat = "released";
		break;
	default:
		buttonStat = "unknown";
		break;
	}

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
		hyperbolicGraph.SetMouseClickStartPos(HyperbolicField::DeviceToHyperbolicPosition(cX, cY));
		break;
	case GLUT_MIDDLE_BUTTON:
		printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
		break;
	case GLUT_RIGHT_BUTTON:
		printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);

		break;
	}
}
void onIdle()
{
	long time = glutGet(GLUT_ELAPSED_TIME);

	if(simulationStarted)	{
		hyperbolicGraph.Simulate();
	}
}
