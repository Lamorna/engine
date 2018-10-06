#pragma once

struct component_data_;
struct way_point_manager_;
struct behaviour_manager_;
struct animation_manager_;
struct model_manager_;
struct model_token_;
struct draw_call_;
struct grid_;
struct memory_chunk_;

void COMPONENT_Populate_Table(

	component_data_&,
	way_point_manager_&,
	behaviour_manager_&,
	animation_manager_&,
	model_manager_&,
	model_token_[],
	draw_call_[],
	grid_&,
	memory_chunk_&
);