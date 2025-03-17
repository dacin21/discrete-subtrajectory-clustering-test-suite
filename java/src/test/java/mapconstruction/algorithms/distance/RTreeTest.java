package mapconstruction.algorithms.distance;

import junit.framework.TestCase;

import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.util.*;

public class RTreeTest extends TestCase {

    public void testInsert1() {
        RTree<Line2D, Integer> rTree = new RTree<>(2);

        rTree.insert(new Line2D.Double(-2, 0, 0, -2), 1);
        rTree.insert(new Line2D.Double(-1, 0, 0, 0), 2);
        rTree.insert(new Line2D.Double(2, -1, 2, 0), 3);

        ArrayList<Integer> expected = new ArrayList<>();
        expected.add(1);
        expected.add(2);
        expected.add(3);

        // BUG: rTree.values() is not properly implemented
        Collection<Integer> actual = rTree.values(); // fails as the root is not a leaf node anymore.
        // Collection<Integer> actual = rTree.windowQuery(-10, -10, 10, 10); // works

        System.out.println("size: " + rTree.size());
        System.out.println("actual: " + actual);

        TestCase.assertTrue("All values are inserted", actual.containsAll(expected));
    }

    public void testWindow1() {
        RTree<Line2D, Integer> rTree = new RTree<>(2);

        rTree.insert(new Line2D.Double(-2, 0, 0, -2), 1);
        rTree.insert(new Line2D.Double(-1, 0, 0, 0), 2);
        rTree.insert(new Line2D.Double(2, -1, 2, 0), 3);

        Set<Integer> expected = new HashSet<>();
        expected.add(1);
        expected.add(2);

        Set<Integer> actual = rTree.windowQuery(-1, -1, 1, 1);

        TestCase.assertEquals(expected, actual);
    }

    public void testBounds1() {
        RTree<Line2D, Integer> rTree = new RTree<>(2);

        rTree.insert(new Line2D.Double(1, 1, 2, 2), 1);
        rTree.insert(new Line2D.Double(1, 2, 2, 2), 2);
        rTree.insert(new Line2D.Double(3, 1, 2, 2), 3);

        Rectangle2D expected = new Rectangle2D.Double(1, 1, 2, 1);

        Rectangle2D actual = rTree.getBounds();

        TestCase.assertEquals(expected, actual);
    }

}
