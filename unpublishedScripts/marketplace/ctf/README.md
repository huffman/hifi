### ballClientEntity.js

  * Keeps track of whether it is being held locally.
  * Has a message interface to release the ball. This message is sent when the
      portal gun bullet/orb hits the user.
  * Releases the ball when the goal is touched, and sends a message to the goal

### goalClientEntity.js

 * Shoots fireworks when the ball is scored
 * Manages a cooldown so the goal can only be triggered every 2 seconds (on the client, so technically if 2 clients
   scored in a shorted period of time it would still trigger twice).

### portalGunClientEntity.js

Portal gun client entity script

### portalBulletClientEntity.js

Portal bullet client entity script. Teleports a user to the starting area when it hits a user

### gunSpawnerClientEntity.js

Spawns guns when it is clicked/grabbed/fargrabbed
