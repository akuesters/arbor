.. _tutorial:

Build single cell model
=======================

The objectives of this step-by-step walk through are to get familiar with the basic operations of Arbor using Python. We will

- import the arbor module (:ref:`tut_step1`);
- describe a single cell model (:ref:`tut_step2`);
- describe the computational resources (:ref:`tut_step3`);
- partition the model over the hardware resources (:ref:`tut_step4`);
- set up recording of spikes and voltages (:ref:`tut_step5`);
- run the model (:ref:`tut_step6`);
- visualize the results (:ref:`tut_step7`);

Prerequisites
~~~~~~~~~~~~~

Once Arbor has been built and installed (see the :ref:`installation guide <installarbor>`),
the location of the installed module needs to be set in the ``PYTHONPATH`` environment variable using the terminal.
For example:

.. code-block:: bash

    export PYTHONPATH="/usr/lib/python3.7/site-packages/:$PYTHONPATH"

.. _tut_step1:

Step 1: Import Arbor Module
~~~~~~~~~~~~~~~~~~~~~~~~~~~

With this setup, Arbor's python module :py:mod:`arbor` can be imported with python3. Either start python in the terminal or create a .py file and type:

    .. code-block:: python3

        import arbor

With :func:`arbor.config`, we get Arbor's version number:

    .. code-block:: python3

        arbor.config()['version']

Additionally, we can check how Arbor was configured at compile time:

    .. code-block:: python3

        arbor.config()['mpi']
        arbor.config()['mpi4py']
        arbor.config()['gpu']

.. _tut_step2:

Step 2: Build a simple cell
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _tut_step3:

Step 3: Describe computational resources
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
As a third step, we need to create a local execution context using :func:`arbor.context`

    .. code-block:: python3

        # Construct a context using 1 thread and no GPU or MPI
        context = arbor.context()

        # Construct a context that:
        #  * uses 8 threads in its thread pool;
        #  * does not use a GPU, reguardless of whether one is available
        #  * does not use MPI.
        context = arbor.context(threads = 8, gpu_id = None, mpi = None)

.. _tut_step4:

Step 4: Partition the model over resources
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _tut_step5:

Step 5: Record spikes and voltage traces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _tut_step6:

Step 6: Run the simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _tut_step7:

Step 7: Visualize the results
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
