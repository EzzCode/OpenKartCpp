import json

# Define the base wall entity
base_wall = {
    "name": "wall4",
    "position": [183, 4, 10],#This will be updated in the loop
    "rotation": [90,90,0],
    "scale": [10, 10, 10],
    "components": [
        {
            "type": "Mesh Renderer",
            "mesh": "plane",
            "material": "glass"
        },
        {
            "type": "Collider",
            "x_diff": 1,
            "y_diff": 0.06,
            "z_diff": 1,
            "action": 1
        }
    ]
}

# Generate the walls
walls = []
start_position_z = 183
num_walls = 20
z_increment = -10

for i in range(num_walls):
    wall = base_wall.copy()
    wall["position"] = [start_position_z + i * z_increment, 4,10 ]
    walls.append(wall)

# Convert to JSON
json_output = json.dumps(walls, indent=4)

# Print or save the JSON output
print(json_output)

# Optionally, save to a file
with open("wall.txt", "w") as f:
    f.write(json_output)