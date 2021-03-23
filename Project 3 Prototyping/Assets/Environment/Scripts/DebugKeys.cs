using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class DebugKeys : MonoBehaviour
{

    public Transform[] teleportTarget;
    public GameObject player;
    public GameObject camera;
    int i = 0;
    // Start is called before the first frame update
    void Start()
    {
       
        
    }

    // Update is called once per frame
    void Update()
    {

        if (Input.GetKeyDown(KeyCode.F1))
        {
            i += 1;
            player.transform.position = teleportTarget[i].transform.position;
            camera.transform.position = teleportTarget[i].transform.position;
        }

        if (Input.GetKeyDown(KeyCode.F2))
        {
            i -= 1;
            player.transform.position = teleportTarget[i].transform.position;
            camera.transform.position = teleportTarget[i].transform.position;
            
        }

        if (i < 0)
        {
            i = 0;
        }

        if (Input.GetKeyDown(KeyCode.F11))
        {
            SceneManager.LoadScene(("Level 1 Blockout"));
        }

        if (Input.GetKeyDown(KeyCode.F12))
        {
            SceneManager.LoadScene(("Level 2 Blockout"));
        }



        //if (Input.GetKeyDown(KeyCode.F1))
        //{
        //    player.transform.position = teleportTarget[0].transform.position;
        //    camera.transform.position = teleportTarget[0].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F2))
        //{
        //    player.transform.position = teleportTarget[1].transform.position;
        //    camera.transform.position = teleportTarget[1].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F3))
        //{
        //    player.transform.position = teleportTarget[2].transform.position;
        //    camera.transform.position = teleportTarget[2].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F4))
        //{
        //    player.transform.position = teleportTarget[3].transform.position;
        //    camera.transform.position = teleportTarget[3].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F5))
        //{
        //    player.transform.position = teleportTarget[4].transform.position;
        //    camera.transform.position = teleportTarget[4].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F6))
        //{
        //    player.transform.position = teleportTarget[5].transform.position;
        //    camera.transform.position = teleportTarget[5].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F7))
        //{
        //    player.transform.position = teleportTarget[6].transform.position;
        //    camera.transform.position = teleportTarget[6].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F8))
        //{
        //    player.transform.position = teleportTarget[7].transform.position;
        //    camera.transform.position = teleportTarget[7].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F9))
        //{
        //    player.transform.position = teleportTarget[8].transform.position;
        //    camera.transform.position = teleportTarget[8].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F10))
        //{
        //    player.transform.position = teleportTarget[9].transform.position;
        //    camera.transform.position = teleportTarget[9].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F11))
        //{
        //    player.transform.position = teleportTarget[10].transform.position;
        //    camera.transform.position = teleportTarget[10].transform.position;
        //}

        //if (Input.GetKeyDown(KeyCode.F12))
        //{
        //    SceneManager.LoadScene(("Level 2 Blockout"));
        //}
    }

}
