
## To do list

### 1

#### Problem description

The rendering part is very laggy when the number of nodes reach 100.

To reproduce the issue,  
Create an empty level and then
drag the cpp class KnowledgeGraph to the level.


Then set the number of nodes to 300  (jnode).

connect_to_previous should be unchecked.

![image](https://github.com/user-attachments/assets/ee1ed9fe-dc6d-421e-9d99-f05157af9baf)



Then run the game.

You will find that the frame rate is very low, less than 10 fps.




#### Possible solution
Number 5 is chosen.

1. Need to time the exact time each function in order to find the bottleneck
2. Use the profiler to find the bottleneck
3. The reason of the laggy rendering is possibly because it takes too much time to calculate the charge force.
   my implementation is actually Reference from the javascript implementation https://github.com/vasturiano/three-forcegraph.
   the javascript version could render 1000 or few thousand nodes in real time smoothly.
   In the javascript implementation of the OcTree , there is no Pointer. The location information of all the nodes
   is directly stored in an array With eight elements, Each elements Is either NULL Or an array with eight elements, so on and so forth.
   Perhaps this is the reason why the javascript implementation is faster than mine.
   The way that I implement the tree is basically every node have 8 pointers, and every pointer Is either a null pointer or a pointer to another tree node. I am unsure whether this structure will prolong the calculation.

4. Use some multi threading method to speed up the calculation

5. Use compute shader


### 2

Applying an example in Github.
Which is a shader to calculate Many body force in two dimensional.
We want to change it to three dimensional.

from Many examples In Google searches, we observe that the GPU calculation Could be Trigger in two ways.
1. OnPostResolvedSceneColorHandle =
   RendererModule->
   GetResolvedSceneColorCallbacks()
   .AddRaw(this,
   &XXXXXXXXXXXX::YYYYYYYYYY_RenderThread
   );
2. manually triggered By using dispatch,
   Passing in a callback function so that when gpu calculation is done, the callback function will be called.

I wonder if there are a way can do it in a synchronous block way,
https://github.com/MatthisL/UE5_NBodySimulation/issues/1,
But I will assume now it is very difficult and not efficient to do it in this way.

So it seems we only have two main ways. But since I do not know how to use The callback function properly.
I will try to use the first way.
Because if we want to use the first way, I suggest we only have one shader, because if we have multiple shaders,
I'm not sure how they could execute in the correct order.
Because I want to ensure that we always apply the link force first,
and then the many body force,
and then the center force.

But how could we compute the 3 forces in a single shader?
In this function in this commit https://github.com/ProgramIsFun/ue5-force-graphhhhhhhhh-521/blob/50ce2f2684cabef0576102ec612634937be007d5/Plugins/NBodySimShader/Shaders/Private/NBodySim.usf#L69
Only the many body force is computed.
Perhaps we could add the link force and the center force in this function.
However, because the way that we are dispatching the shader,
SV_DispatchThreadID Of any specific thread is likely to be not larger than the number of bodies.
The number of links in the graph could be much larger than number of bodies.
So a thread With a given SV_DispatchThreadID,
Could be responsible for computing all the connected nodes of a specific node With that ID.
So let's say the node ID 3 Has a lot of connecting nodes,
Then the thread With SV_DispatchThreadID 3 will be responsible for computing all the connected nodes of node 3.
This might be a bad idea, because some thread might be computing with more times than
other threads on the ID that have very less connected nodes.

For the center force, This could be ignored for now.





### 3

Try to run the shaders when there are only two elements to see whether they compute the value correctly.

Perhaps need to add additional debugger of the shader, because sometimes it returns NAN.

### 4

add the link force


We can either
1. Deal with the lake forest in cpu and past results to the shade by overwriting the velocity.




We need to be able to retrieve the position and also the velocity array.
we will change the position and velocity And write it as a parameter and update the shader.

The question is shall we update the velocity thing to the shader
before We get the position, or after we get the position.

Because the shader will compute the position force according to the velocity, perhaps that should be done before.

And because the shader will update The position according to the velocity, perhaps we should also pass The Alpha value to the shader.


does the shade only Once, if we set the TICK interval to be very large.

The result is If we set the TICK interval to maybe perhaps 1 second, it runs a few hundred times already.
The possible reasons is The callback function is called every frame.  So the shader is called every frame.
Right now, we could assume that if we do not set the interval, and then it will run exactly once between each tick.

Second thing is Let's say we update a variable. Does the result immediately result from the update value or the previous update


Let's also test can we overwrite the position directly in the shader
It seems pretty hard because in every initialized phrase, if the buffer is already there, we will not overwritten it.
Perhaps could reference the following project.

https://github.com/UE4-OceanProject/ComputeShaderDev

https://forums.unrealengine.com/t/loading-data-to-from-structured-buffer-compute-shaders/470083/2

The above seem pretty complicated.




2. Or we could compute the link force in the shader.


GPT give me two suggestions on computing the link force.

2.1

StructuredBuffer<uint> LinkIndices;                // Flatten 2D array of indices
const uint MaxLinksPerBody;                        // Maximum number of linked bodies per node

// Link forces calculation
for (uint j = 0; j < MaxLinksPerBody; j++)
{
uint LinkedBodyIndex = LinkIndices[ID.x * MaxLinksPerBody + j];

        Check for sentinel value indicating no more connections
        
        if (LinkedBodyIndex == UINT_MAX) 
            break;  // This stops processing further links for this node

        float2 LinkDirection = normalize(Positions[LinkedBodyIndex] - Positions[ID.x]);
        float LinkStrength = 1.0f;  // Placeholder strength
        Acceleration += LinkDirection * LinkStrength;
}

2.2

Offset/Count Buffer: Each entry contains a starting index and the count of connections for the corresponding body.
Links Buffer: A flat buffer that contains all the links in a single array, sequenced as per the offsets and counts indicated in the first buffer.
Shader Code Adjustment:
StructuredBuffer<uint2> LinkInfo;       // Holds pairs (offset, count) for each body
StructuredBuffer<uint> LinkIndices;     // Flat array containing all links

// In your compute shader
for (uint j = 0; j < LinkInfo[ID.x].y; j++) // LinkInfo[ID.x].y gives the count of links
{
uint index = LinkInfo[ID.x].x + j;  // Starting index + offset
uint LinkedBodyIndex = LinkIndices[index];
// Calculate forces similar to previous examples
}



2.3

StructuredBuffer<uint> LinkOffsets;  // Holds the offset for each body
StructuredBuffer<uint> LinkCounts;   // Holds the count of links for each body
StructuredBuffer<uint> LinkIndices;  // Flat array containing all links
StructuredBuffer<float> LinkStrengths;  // Holds the strength of each link
StructuredBuffer<float> LinkBiases;     // Holds the bias of each link

[numthreads(x, 1, 1)]
void ComputeShader(uint3 ID : SV_DispatchThreadID)
{
uint linkCount = LinkCounts[ID.x];  // Get count of links
uint offset = LinkOffsets[ID.x];    // Get starting offset

    for (uint j = 0; j < linkCount; j++)
    {
        uint index = offset + j;  // Compute global index in the LinkIndices
        uint LinkedBodyIndex = LinkIndices[index]; // Get the linked body index

        // Process further using LinkedBodyIndex, 
        // LinkStrengths[index], LinkBiases[index] etc.

}





This perhaps explain why flatten 1D array is better than 2D array.
https://stackoverflow.com/questions/19759343/cuda-is-it-better-to-use-m-for-2d-static-arrays-or-flatten-them-to-m


To prevent race condition when calculating the link force in the shader, perhaps each thread will only deal with the change of position or velocity of that node only.
So one link could affect 2 Threats.

So we compute a flattened one D away, which store the bias of each link.
Another flattened 1D array calculate the strength of each link.

It seems that having too many array of the same size is not very good, right?
Perhaps need to find an example which use a buffer of a custom struct.







### 5
It take too much time for generating too much actor for each link.
We use components instead of an actor for each link.



### 6
I believe the default PAWN is hard to customize, 
create a custom characters. 



### 7
Get the location of the characters and update the TEXT render component rotation. 

Perhaps we could download some first person characters Example to see how it works. 

- Try the official sample project
  - C:\Users\whouse\Desktop\unreal\ue521fpscale37
    - https://github.com/everythingallaccount/ue521fpscale37
  - C:\Users\whouse\Desktop\unreal\ue521fpscale37bp
    - https://github.com/everythingallaccount/ue521fpscale37bp 

- https://www.youtube.com/watch?v=SovEiHzHlqU&ab_channel=HarrisonMcGuire

### 8

Deal with import graph function

Accept JSON formats


### 9

Add a spawn graph or delete graph function

Perhaps there are some buttons in the left or upper right corner

Also add some buttons that might change the size of the node in the graph

### 10

Add an import graph function, which the file following, following format. 

```
{
    "nodes": [
        {
            "id": "c2832bf9-e2b4-4b6c-8654-be2c9a8a145a",
            "name": "node_0",
            "__indexColor": "#ec0001",
            "__bckgDimensions": [
                13.336030754936925,
                4.535715779621544
            ],
            "index": 0
        },
        {
            "id": "a316c7c1-8048-4d87-8d12-656377de6cfe",
            "name": "node_1",
            "__indexColor": "#d80002",
            "index": 1,
            "__bckgDimensions": [
                13.336030754936925,
                4.535715779621544
            ]
        }
    ],
    "links": [
        {
            "id": "9133370a-05c3-4dee-a192-d2b3eb22368f",
            "name": "link_9133370a-05c3-4dee-a192-d2b3eb22368f",
            "__indexColor": "#c40003",
            "__photons": [
                {
                    "__progressRatio": 0.3320000000000009
                },
                {
                    "__progressRatio": 0.5820000000000011
                },
                {
                    "__progressRatio": 0.8320000000000011
                },
                {
                    "__progressRatio": 0.08200000000000095
                }
            ],
            "__controlPoints": null,
            "index": 0,
            "source": "c2832bf9-e2b4-4b6c-8654-be2c9a8a145a",
            "target": "a316c7c1-8048-4d87-8d12-656377de6cfe"
        }
    ]
}
```