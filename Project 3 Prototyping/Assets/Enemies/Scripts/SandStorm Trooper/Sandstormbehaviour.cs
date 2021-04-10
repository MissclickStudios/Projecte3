using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Sandstormbehaviour : MonoBehaviour
{
    private float fireRate;
    private float attackDuration = 2.0f;
    private float reloadDuration = 2.0f;
    private bool reloading;
    private bool trigerredAttack;
    public GameObject aimingTarget;
    public float TriggerDistance = 10.0f;
    public GameObject Bullet;
    public GameObject target;
    public float sAttack1BulletSpeed = 150;
    public float movementSpeed;
    private float speed;

    // Start is called before the first frame update
    void Start()
    {
        trigerredAttack = false;
        reloading = false;
    }

    // Update is called once per frame
    void Update()
    {
        target.transform.position = new Vector3(aimingTarget.transform.position.x, aimingTarget.transform.position.y + 1f, aimingTarget.transform.position.z);

        attackDuration -= Time.deltaTime;
        Aim();
        movement();
        if (reloading != true)
        {
            if (trigerredAttack == true)
            {
                Shoot();
            }

            if (attackDuration < 0.0f)
            {
                reloadDuration = 2.0f;

                reloading = true;
            }
        }

        if (reloading == true)
        {
            reloadDuration -= Time.deltaTime;

            if (reloadDuration < 0.0f)
            {
                attackDuration = 5.0f;
                reloading = false;
            }
        }

    }
    void Aim()
    {
        transform.LookAt(target.transform.position);

        if ((transform.position - target.transform.position).magnitude < TriggerDistance && trigerredAttack == false)
        {

            trigerredAttack = true;

        }
    }
    void Shoot()
    {
        fireRate += Time.deltaTime;

        if (fireRate >= 1.0f)
        {
            GameObject myBulletPrefab = Instantiate(Bullet, transform.position, Quaternion.identity) as GameObject;
            Rigidbody myBulletPrefabRigidBody = myBulletPrefab.GetComponent<Rigidbody>();
            myBulletPrefabRigidBody.AddForce(transform.forward * sAttack1BulletSpeed);

            //GameObject myBulletPrefab2 = Instantiate(Bullet, transform.position, Quaternion.identity) as GameObject;
            //Rigidbody myBulletPrefabRigidBody2 = myBulletPrefab2.GetComponent<Rigidbody>();
            //myBulletPrefabRigidBody2.AddForce(transform.forward * sAttack1BulletSpeed);

            fireRate = 0;
            trigerredAttack = false;
        }

    }
    void movement()
    {
        speed = Time.deltaTime * movementSpeed;

        if (Vector3.Distance(transform.position, aimingTarget.transform.position) <= 10f && Vector3.Distance(transform.position, aimingTarget.transform.position) >= 4f)
        {
            transform.position = Vector3.MoveTowards(transform.position, new Vector3(aimingTarget.transform.position.x, 2f, aimingTarget.transform.position.z), speed);
            
        }

    }
}
