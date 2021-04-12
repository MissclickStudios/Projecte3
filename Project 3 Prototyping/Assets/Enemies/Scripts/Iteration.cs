using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class Iteration : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.F1))
        {
            SceneManager.LoadScene(("Trooper Design"));
        }

        if (Input.GetKeyDown(KeyCode.F2))
        {
            SceneManager.LoadScene(("Blurrg Design"));
        }

        if (Input.GetKeyDown(KeyCode.F3))
        {
            SceneManager.LoadScene(("Turret Design"));
        }

        if (Input.GetKeyDown(KeyCode.F4))
        {
            SceneManager.LoadScene(("Grogu Design"));
        }

        if (Input.GetKeyDown(KeyCode.F5))
        {
            SceneManager.LoadScene(("Boss Design"));
        }

        if (Input.GetKeyDown(KeyCode.F6))
        {
            SceneManager.LoadScene(("Boss Design Raged"));
        }

        if (Input.GetKeyDown(KeyCode.Escape))
        {
            Application.Quit();
        }

    }
}
