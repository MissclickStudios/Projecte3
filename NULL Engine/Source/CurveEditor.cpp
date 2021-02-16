#include "CurveEditor.h"

int ImGui::CurveEditor(const char* label, float* values, int pointsCount, const ImVec2& editorSize, ImU32 flags, int* newCount)
{
	const float HEIGHT = 100;
	static ImVec2 startPan;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	ImVec2 size = editorSize;
	size.x = size.x < 0 ? CalcItemWidth() + (style.FramePadding.x * 2) : size.x;
	size.y = size.y < 0 ? HEIGHT : size.y;

	ImGuiWindow* parentWindow = GetCurrentWindow();
	ImGuiID id = parentWindow->GetID(label);
	if (!BeginChildFrame(id, size, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		EndChild();
		return -1;
	}

	int hoveredIdx = -1;
	if (newCount) *newCount = pointsCount;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
	{
		EndChild();
		return -1;
	}

	ImVec2 pointsMin(FLT_MAX, FLT_MAX);
	ImVec2 pointsMax(-FLT_MAX, -FLT_MAX);
	for (int pointIdx = 0; pointIdx < pointsCount; ++pointIdx)
	{
		ImVec2 point;
		if (flags & (int)CURVE_EDITOR_FLAGS::NO_TANGENTS)
		{
			point = ((ImVec2*)values)[pointIdx];
		}
		else
		{
			point = ((ImVec2*)values)[1 + pointIdx * 3];
		}
		pointsMax = ImMax(pointsMax, point);
		pointsMin = ImMin(pointsMin, point);
	}
	pointsMax.y = ImMax(pointsMax.y, pointsMin.y + 0.0001f);

	float fromX = window->StateStorage.GetFloat((ImGuiID)STORAGE_VALUES::FROM_X, pointsMin.x);
	float fromY = window->StateStorage.GetFloat((ImGuiID)STORAGE_VALUES::FROM_Y, pointsMin.y);
	float width = window->StateStorage.GetFloat((ImGuiID)STORAGE_VALUES::WIDTH, pointsMax.x - pointsMin.x);
	float height = window->StateStorage.GetFloat((ImGuiID)STORAGE_VALUES::HEIGHT, pointsMax.y - pointsMin.y);
	window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::FROM_X, fromX);
	window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::FROM_Y, fromY);
	window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::WIDTH, width);
	window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::HEIGHT, height);

	ImVec2 begPos = GetCursorScreenPos();

	const ImRect innerBb = window->InnerRect;
	const ImRect frameBb(innerBb.Min - style.FramePadding, innerBb.Max + style.FramePadding);

	auto transform = [&](const ImVec2& pos) -> ImVec2
	{
		float x = (pos.x - fromX) / width;
		float y = (pos.y - fromY) / height;

		return ImVec2(
			innerBb.Min.x * (1 - x) + innerBb.Max.x * x,
			innerBb.Min.y * y + innerBb.Max.y * (1 - y)
		);
	};

	auto invTransform = [&](const ImVec2& pos) -> ImVec2
	{
		float x = (pos.x - innerBb.Min.x) / (innerBb.Max.x - innerBb.Min.x);
		float y = (innerBb.Max.y - pos.y) / (innerBb.Max.y - innerBb.Min.y);

		return ImVec2(
			fromX + width * x,
			fromY + height * y
		);
	};

	if (flags & (int)CURVE_EDITOR_FLAGS::SHOW_GRID)
	{
		int exp;
		frexp(width / 5, &exp);
		float stepX = (float)ldexp(1.0, exp);
		int cellCols = int(width / stepX);

		float x = stepX * int(fromX / stepX);
		for (int i = -1; i < cellCols + 2; ++i)
		{
			ImVec2 a = transform({ x + i * stepX, fromY });
			ImVec2 b = transform({ x + i * stepX, fromY + height });
			window->DrawList->AddLine(a, b, 0x55000000);
			char buf[64];
			if (exp > 0)
			{
				ImFormatString(buf, sizeof(buf), " %d", int(x + i * stepX));
			}
			else
			{
				ImFormatString(buf, sizeof(buf), " %f", x + i * stepX);
			}
			window->DrawList->AddText(b, 0x55000000, buf);
		}

		frexp(height / 5, &exp);
		float stepY = (float)ldexp(1.0, exp);
		int cellRows = int(height / stepY);

		float y = stepY * int(fromY / stepY);
		for (int i = -1; i < cellRows + 2; ++i)
		{
			ImVec2 a = transform({ fromX, y + i * stepY });
			ImVec2 b = transform({ fromX + width, y + i * stepY });
			window->DrawList->AddLine(a, b, 0x55000000);
			char buf[64];
			if (exp > 0)
			{
				ImFormatString(buf, sizeof(buf), " %d", int(y + i * stepY));
			}
			else
			{
				ImFormatString(buf, sizeof(buf), " %f", y + i * stepY);
			}
			window->DrawList->AddText(a, 0x55000000, buf);
		}
	}

	if (ImGui::GetIO().MouseWheel != 0 && ImGui::IsItemHovered())
	{
		float scale = powf(2, ImGui::GetIO().MouseWheel);
		width *= scale;
		height *= scale;
		window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::WIDTH, width);
		window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::HEIGHT, height);
	}
	if (ImGui::IsMouseReleased(1))
	{
		window->StateStorage.SetBool((ImGuiID)STORAGE_VALUES::IS_PANNING, false);
	}
	if (window->StateStorage.GetBool((ImGuiID)STORAGE_VALUES::IS_PANNING, false))
	{
		ImVec2 dragOffset = ImGui::GetMouseDragDelta(1);
		fromX = startPan.x;
		fromY = startPan.y;
		fromX -= dragOffset.x * width / (innerBb.Max.x - innerBb.Min.x);
		fromY += dragOffset.y * height / (innerBb.Max.y - innerBb.Min.y);
		window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::FROM_X, fromX);
		window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::FROM_Y, fromY);
	}
	else if (ImGui::IsMouseDragging(1) && ImGui::IsItemHovered())
	{
		window->StateStorage.SetBool((ImGuiID)STORAGE_VALUES::IS_PANNING, true);
		startPan.x = fromX;
		startPan.y = fromY;
	}

	int changedIdx = -1;
	for (int pointIdx = pointsCount - 2; pointIdx >= 0; --pointIdx)
	{
		ImVec2* points;
		if (flags & (int)CURVE_EDITOR_FLAGS::NO_TANGENTS)
		{
			points = ((ImVec2*)values) + pointIdx;
		}
		else
		{
			points = ((ImVec2*)values) + 1 + pointIdx * 3;
		}

		ImVec2 pPrev = points[0];
		ImVec2 tangentLast;
		ImVec2 tangent;
		ImVec2 p;
		if (flags & (int)CURVE_EDITOR_FLAGS::NO_TANGENTS)
		{
			p = points[1];
		}
		else
		{
			tangentLast = points[1];
			tangent = points[2];
			p = points[3];
		}

		auto handlePoint = [&](ImVec2& p, int idx) -> bool
		{
			static const float SIZE = 3;

			ImVec2 cursorPos = GetCursorScreenPos();
			ImVec2 pos = transform(p);

			SetCursorScreenPos(pos - ImVec2(SIZE, SIZE));
			PushID(idx);
			InvisibleButton("", ImVec2(2 * nodeSlotRadius, 2 * nodeSlotRadius));

			ImU32 col = IsItemActive() || IsItemHovered() ? GetColorU32(ImGuiCol_PlotLinesHovered) : GetColorU32(ImGuiCol_PlotLines);

			window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, SIZE), col);
			window->DrawList->AddLine(pos + ImVec2(SIZE, 0), pos + ImVec2(0, SIZE), col);
			window->DrawList->AddLine(pos + ImVec2(SIZE, 0), pos + ImVec2(0, -SIZE), col);
			window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, -SIZE), col);

			if (IsItemHovered()) hoveredIdx = pointIdx + idx;

			bool changed = false;
			if (IsItemActive() && IsMouseClicked(0))
			{
				window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::POINT_START_X, pos.x);
				window->StateStorage.SetFloat((ImGuiID)STORAGE_VALUES::POINT_START_Y, pos.y);
			}

			if (IsItemHovered() || IsItemActive() && IsMouseDragging(0))
			{
				char tmp[64];
				ImFormatString(tmp, sizeof(tmp), "%0.2f, %0.2f", p.x, p.y);
				window->DrawList->AddText({ pos.x, pos.y - GetTextLineHeight() }, 0xff000000, tmp);
			}

			if (IsItemActive() && IsMouseDragging(0))
			{
				pos.x = window->StateStorage.GetFloat((ImGuiID)STORAGE_VALUES::POINT_START_X, pos.x);
				pos.y = window->StateStorage.GetFloat((ImGuiID)STORAGE_VALUES::POINT_START_Y, pos.y);
				pos += ImGui::GetMouseDragDelta();
				ImVec2 v = invTransform(pos);

				p = v;
				changed = true;
			}
			PopID();

			SetCursorScreenPos(cursorPos);
			return changed;
		};

		auto handleTangent = [&](ImVec2& t, const ImVec2& p, int idx) -> bool
		{
			static const float SIZE = 2;
			static const float LENGTH = 18;

			auto normalized = [](const ImVec2& v) -> ImVec2
			{
				float len = 1.0f / sqrtf(v.x * v.x + v.y * v.y);
				return ImVec2(v.x * len, v.y * len);
			};

			ImVec2 cursorPos = GetCursorScreenPos();
			ImVec2 pos = transform(p);
			ImVec2 tang = pos + normalized(ImVec2(t.x, -t.y)) * LENGTH;

			SetCursorScreenPos(tang - ImVec2(SIZE, SIZE));
			PushID(-idx);
			InvisibleButton("", ImVec2(2 * nodeSlotRadius, 2 * nodeSlotRadius));

			window->DrawList->AddLine(pos, tang, GetColorU32(ImGuiCol_PlotLines));

			ImU32 col = IsItemHovered() ? GetColorU32(ImGuiCol_PlotLinesHovered) : GetColorU32(ImGuiCol_PlotLines);

			window->DrawList->AddLine(tang + ImVec2(-SIZE, SIZE), tang + ImVec2(SIZE, SIZE), col);
			window->DrawList->AddLine(tang + ImVec2(SIZE, SIZE), tang + ImVec2(SIZE, -SIZE), col);
			window->DrawList->AddLine(tang + ImVec2(SIZE, -SIZE), tang + ImVec2(-SIZE, -SIZE), col);
			window->DrawList->AddLine(tang + ImVec2(-SIZE, -SIZE), tang + ImVec2(-SIZE, SIZE), col);

			bool changed = false;
			if (IsItemActive() && IsMouseDragging(0))
			{
				tang = GetIO().MousePos - pos;
				tang = normalized(tang);
				tang.y *= -1;

				t = tang;
				changed = true;
			}
			PopID();

			SetCursorScreenPos(cursorPos);
			return changed;
		};

		PushID(pointIdx);
		if ((flags & (int)CURVE_EDITOR_FLAGS::NO_TANGENTS) == 0)
		{
			window->DrawList->AddBezierCurve(
				transform(pPrev),
				transform(pPrev + tangentLast),
				transform(p + tangent),
				transform(p),
				GetColorU32(ImGuiCol_PlotLines),
				1.0f,
				20);
			if (handleTangent(tangentLast, pPrev, 0))
			{
				points[1] = ImClamp(tangentLast, ImVec2(0, -1), ImVec2(1, 1));
				changedIdx = pointIdx;
			}
			if (handleTangent(tangent, p, 1))
			{
				points[2] = ImClamp(tangent, ImVec2(-1, -1), ImVec2(0, 1));
				changedIdx = pointIdx + 1;
			}
			if (handlePoint(p, 1))
			{
				if (p.x <= pPrev.x) p.x = pPrev.x + 0.001f;
				if (pointIdx < pointsCount - 2 && p.x >= points[6].x)
				{
					p.x = points[6].x - 0.001f;
				}
				points[3] = p;
				changedIdx = pointIdx + 1;
			}

		}
		else
		{
			window->DrawList->AddLine(transform(pPrev), transform(p), GetColorU32(ImGuiCol_PlotLines), 1.0f);
			if (handlePoint(p, 1))
			{
				if (p.x <= pPrev.x) p.x = pPrev.x + 0.001f;
				if (pointIdx < pointsCount - 2 && p.x >= points[2].x)
				{
					p.x = points[2].x - 0.001f;
				}
				points[1] = p;
				changedIdx = pointIdx + 1;
			}
		}
		if (pointIdx == 0)
		{
			if (handlePoint(pPrev, 0))
			{
				if (p.x <= pPrev.x) pPrev.x = p.x - 0.001f;
				points[0] = pPrev;
				changedIdx = pointIdx;
			}
		}
		PopID();
	}

	SetCursorScreenPos(innerBb.Min);

	InvisibleButton("bg", innerBb.Max - innerBb.Min);

	if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0) && newCount)
	{
		ImVec2 mp = ImGui::GetMousePos();
		ImVec2 new_p = invTransform(mp);
		ImVec2* points = (ImVec2*)values;

		if ((flags & (int)CURVE_EDITOR_FLAGS::NO_TANGENTS) == 0)
		{
			points[pointsCount * 3 + 0] = ImVec2(-0.2f, 0);
			points[pointsCount * 3 + 1] = new_p;
			points[pointsCount * 3 + 2] = ImVec2(0.2f, 0);;
			++* newCount;

			auto compare = [](const void* a, const void* b) -> int
			{
				float fa = (((const ImVec2*)a) + 1)->x;
				float fb = (((const ImVec2*)b) + 1)->x;
				return fa < fb ? -1 : (fa > fb) ? 1 : 0;
			};

			qsort(values, pointsCount + 1, sizeof(ImVec2) * 3, compare);

		}
		else
		{
			points[pointsCount] = new_p;
			++* newCount;

			auto compare = [](const void* a, const void* b) -> int
			{
				float fa = ((const ImVec2*)a)->x;
				float fb = ((const ImVec2*)b)->x;
				return fa < fb ? -1 : (fa > fb) ? 1 : 0;
			};

			qsort(values, pointsCount + 1, sizeof(ImVec2), compare);
		}
	}

	if (hoveredIdx >= 0 && ImGui::IsMouseDoubleClicked(0) && newCount && pointsCount > 2)
	{
		ImVec2* points = (ImVec2*)values;
		--* newCount;
		if ((flags & (int)CURVE_EDITOR_FLAGS::NO_TANGENTS) == 0)
		{
			for (int j = hoveredIdx * 3; j < pointsCount * 3 - 3; j += 3)
			{
				points[j + 0] = points[j + 3];
				points[j + 1] = points[j + 4];
				points[j + 2] = points[j + 5];
			}
		}
		else
		{
			for (int j = hoveredIdx; j < pointsCount - 1; ++j)
			{
				points[j] = points[j + 1];
			}
		}
	}

	EndChildFrame();
	RenderText(ImVec2(frameBb.Max.x + style.ItemInnerSpacing.x, innerBb.Min.y), label);
	return changedIdx;
}