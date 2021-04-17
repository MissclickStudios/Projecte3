using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TestKill : MonoBehaviour
{

    ATSTBehaviour refScript;
   
    // Start is called before the first frame update
    void Start()
    {
        refScript = GetComponent<ATSTBehaviour>();
    }

    // Update is called once per frame
    void Update()
    {
    }

    private void OnCollisionEnter(Collision collision)
    {
        if(collision.gameObject.tag == "Grenade")
        {
            _360AttackGrenade();
            Destroy(gameObject);
        }
    }

    //---------------------------------------------------------------------------------
    [Header("360 Grenade Attack Settings")]
    public int grenadeNumProjectiles;
    public float grenadeProjectileSpeed;
    public GameObject _360Projectile;

    private const float radius = 1F;
    public void _360AttackGrenade()
    {
        float angleStep = 360f / grenadeNumProjectiles;
        float angle = 0f;

        for (int i = 1; i <= grenadeNumProjectiles * 2; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = transform.position.x + Mathf.Sin((angle * Mathf.PI) / 180) * radius;
            float projectileDirYPosition = transform.position.y + Mathf.Cos((angle * Mathf.PI) / 180) * radius;

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - transform.position).normalized * grenadeProjectileSpeed;

            GameObject tmpObj = Instantiate(_360Projectile, transform.position, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

            angle += angleStep;

        }
    }
}
