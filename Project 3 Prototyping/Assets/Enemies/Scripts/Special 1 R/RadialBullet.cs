using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RadialBullet : MonoBehaviour
{
    [Header("Projectile Settings")]
    public int numberOfProjectiles;
    public float projectileSpeed;
    public GameObject ProjectilePrefab;

    public BossMovementSpecial1 BossMovement;

    [Header("Private Variables")]
    private Vector3 startPoint;
    private const float radius = 1F;

    private float dt;
    

    // Update is called once per frame
    void Update()
    {
        dt += Time.deltaTime;

        if (BossMovement.ready_for_360 == true)
        {
            if (dt >= 0.5f)
            {
               startPoint = transform.position;
               SpawnProjectile360(numberOfProjectiles);
               dt = 0;
            }
        }
           
    }

    private void SpawnProjectile360(int _numberOfProjectiles)
    {
        float angleStep = 360f / _numberOfProjectiles;
        float angle = 0f;

        for (int i = 1; i <= _numberOfProjectiles *2; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = startPoint.x + Mathf.Sin((angle * Mathf.PI) / 180) * radius;
            float projectileDirYPosition = startPoint.y + Mathf.Cos((angle * Mathf.PI) / 180) * radius;

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * projectileSpeed;

            GameObject tmpObj = Instantiate(ProjectilePrefab, startPoint, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

            angle += angleStep;
        }
    }

}
