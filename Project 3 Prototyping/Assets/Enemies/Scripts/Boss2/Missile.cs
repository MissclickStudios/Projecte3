using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Missile : MonoBehaviour
{
    // launch variables
    [SerializeField] private Transform TargetObjectTF;
    [Range(1.0f, 6.0f)] public float TargetRadius;
    [Range(20.0f, 75.0f)] public float LaunchAngle;

    // state
    private bool bTargetReady;

    // cache
    private Rigidbody rigid;
    private Vector3 initialPosition;
    private Quaternion initialRotation;

    // Use this for initialization
    void Start()
    {
        rigid = GetComponent<Rigidbody>();
        bTargetReady = true;
        initialPosition = transform.position;
        initialRotation = transform.rotation;
        startPoint = transform.position;
    }

    // launches the object towards the TargetObject with a given LaunchAngle
    void Launch() 
    {
        Vector3 projectileXZPos = new Vector3(transform.position.x, 0.0f, transform.position.z);
        Vector3 targetXZPos = new Vector3(TargetObjectTF.position.x, 0.0f, TargetObjectTF.position.z);

        // rotate the object to face the target
        transform.LookAt(targetXZPos);

        // shorthands for the formula
        float R = Vector3.Distance(projectileXZPos, targetXZPos);
        float G = Physics.gravity.y;
        float tanAlpha = Mathf.Tan(LaunchAngle * Mathf.Deg2Rad);
        float H = TargetObjectTF.position.y - transform.position.y;

        // calculate the local space components of the velocity 
        // required to land the projectile on the target object 
        float Vz = Mathf.Sqrt(G * R * R / (2.0f * (H - R * tanAlpha)));
        float Vy = tanAlpha * Vz;

        // create the velocity vector in local space and get it in global space
        Vector3 localVelocity = new Vector3(0f, Vy, Vz);
        Vector3 globalVelocity = transform.TransformDirection(localVelocity);

        // launch the object by setting its initial velocity and flipping its state
        rigid.velocity = globalVelocity;
        bTargetReady = false;
    }

    // Sets a random target around the object based on the TargetRadius
    void SetNewTarget() { bTargetReady = true; }

    // resets the projectile to its initial position
    void ResetToInitialState()
    {
        rigid.velocity = Vector3.zero;
        this.transform.SetPositionAndRotation(initialPosition, initialRotation);
        bTargetReady = false;
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            if (bTargetReady)
            {
                Launch();
            }
            else
            {
                ResetToInitialState();
                SetNewTarget();
            }
        }

        if (Input.GetKeyDown(KeyCode.R))
        {
            ResetToInitialState();
        }
    }
    public int SA1numProjectiles;
    public float SA1projectileSpeed;
    public GameObject SA1projectile;

    private Vector3 startPoint;
    private const float radius = 1F;

    void OnCollisionEnter()
    {
        float angleStep = 360f / 10;
        float angle = 0f;

        for (int i = 1; i <= 10 * 2; i++)
        {
            // Direction Calculation

            float projectileDirXPosition = startPoint.x + Mathf.Sin((angle * Mathf.PI) / 180) * radius;
            float projectileDirYPosition = startPoint.y + Mathf.Cos((angle * Mathf.PI) / 180) * radius;

            Vector3 projectileVector = new Vector3(projectileDirXPosition, projectileDirYPosition, 0);
            Vector3 projectileMoveDirection = (projectileVector - startPoint).normalized * SA1projectileSpeed;

            GameObject tmpObj = Instantiate(SA1projectile, startPoint, Quaternion.identity);
            tmpObj.GetComponent<Rigidbody>().velocity = new Vector3(projectileMoveDirection.x, 0, projectileMoveDirection.y);

            angle += angleStep;

        }
    }

    void OnCollisionExit()
    {
       
    }
}
