extends Node2D

func _ready():
	var example := Example.new()
	print(example.some_optimized_func())
