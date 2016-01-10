Script.include('componentsCreate.js');

var SCENE = {
    name: "home",
    entities: [
        {
            name: "fireplaceSwitch",
            type: "Box",
            position: { x: 0.5, y: 0, z: 0 },
            components: {
                button: {},
                eventProxy: {
                    buttonActivated: {
                        to: 'warpArea',
                        method: 'create',
                        args: ['random']
                    }
                }
            }
        },
        {
            name: "warpArea",
            position: { x: 0, y: 0.5, z: 0 },
            ignoreForCollisions: true,
            components: {
                objectCreator: {}
            }
        },
        {
            name: "fireplace",
            children: [
                {
                    name: 'fireplace.light',
                    type: 'Light',
                    dimensions: { x: 3, y: 3, z: 3 },
                    color: { red: 207, green: 150, blue: 67 },
                    components: {
                        flickeringLight: {}
                    }
                },
            ],
            components: {
                audio: {
                    url: 'http://hifi-public.s3.amazonaws.com/ryan/demo/0619_Fireplace__Tree_B.L.wav',
                    volume: 0.25,
                    loop: true,
                    follow: true
                }
            }
        },

        // // Plane game
        // {
        //     name: "planeGameManager",
        //     components: {
        //         planeGameManager: {
        //         }
        //     },
        //     children: [
        //         {
        //             name: "baton",
        //             type: "Model",
        //             components: {
        //                 planeGameBaton: {
        //                 }
        //             }
        //         }
        //     ]
        // }
    ]
}

Script.scriptEnding.connect(function() {
    print("Script ending");
    destroyScene(SCENE.name);
});

createScene(SCENE);
