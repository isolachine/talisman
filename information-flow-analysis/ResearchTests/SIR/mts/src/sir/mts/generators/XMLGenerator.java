/*
 * Copyright (c) 2006, Benjamin R. Liblit.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     - Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *
 *     - Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 *     - Neither the names of the copyright holders nor the names of
 *       their contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package sir.mts.generators;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.DOMImplementation;
import org.w3c.dom.Document;
import org.w3c.dom.DocumentType;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import sir.mts.Configuration;
import sir.mts.ScriptGenException;
import sir.mts.TestScriptGenerator;

/**
 * <p>Generates an XML encoding of the contents of a STImpL file.</p>
 *
 * @author Benjamin R. Liblit
 * @version 05/03/2006
 */
public final class XMLGenerator implements TestScriptGenerator {

    final class TestCase {
        private final Element element;
        private List comments;
        private List externalSetup;
        private List inlineSetup;
        private Node driver;
        private final List parameters = new ArrayList();
        private Node inputFile;
        private Node outputFile;
        private boolean background;
        private List moves;
        private List inlineFinish;
        private List externalFinish;
        private int paramCount;

        private TestCase(int id) {
             element = newElement("case", "id", "case-" + id);
        }

        private void append(Node parent, Node child) {
            if (child != null) {
                parent.appendChild(child);
            }
        }

        private void append(Node parent, List children) {
            if (children != null) {
                int size = children.size();
                Iterator iterator = children.iterator();
                for (int i = size; i-- > 0; ) {
                    parent.appendChild((Node) iterator.next());
                }
            }
        }

        private void assemble() {
            // setup operations
            if (externalSetup != null || inlineSetup != null) {
                Node setup = newElement("setup");
                append(setup, externalSetup);
                append(setup, inlineSetup);
                append(element, setup);
            }

            Element command = newElement("command");
            if (background) {
                command.setAttribute("background", "true");
            }
            append(command, driver);
            append(command, parameters);
            append(command, inputFile);
            append(command, outputFile);
            element.appendChild(command);

            // finishing operations
            if (externalFinish != null
                    || inlineFinish != null
                    || moves != null) {
                Node finish = newElement("finish");
                append(finish, moves);
                append(finish, inlineFinish);
                append(finish, externalFinish);
                append(element, finish);
            }
        }
    }

    // global information
    private static final DOMImplementation implementation;
    private static final DocumentType docType;

    static {
        try {
            implementation = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder().getDOMImplementation();
        } catch (ParserConfigurationException e) {
            throw new RuntimeException(e);
        }
        docType = implementation.createDocumentType("stimpl", null,
            "../../../../doc/stimpl.dtd");
    }

    // per-generator information
    private Configuration config;

    // per-script information
    private Document doc;
    private Node root;
    private int caseCount;

    // per-test-case information
    private TestCase testCase;

    public XMLGenerator(Configuration config)
            throws ParserConfigurationException {
        this.config = config;
    }

    public void startScript() throws ScriptGenException {
        // build a new top-level document
        doc = implementation.createDocument(null, "stimpl", docType);
        root = doc.getDocumentElement();
        caseCount = 0;
    }

    public int endScript() throws ScriptGenException {
        try {
            // serialize our XML document tree
            TransformerFactory xformFactory = TransformerFactory.newInstance();
            Transformer transformer = xformFactory.newTransformer();
            String systemValue =
                (new File(doc.getDoctype().getSystemId())).getName();
            transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM,
                systemValue);
            Source input = new DOMSource(doc);
            Writer writer = new FileWriter(config.getScriptName());
            Result output = new StreamResult(writer);
            transformer.transform(input, output);
            writer.write('\n');
            writer.flush();
        }
        catch (TransformerException e) {
            throw new ScriptGenException(e);
        }
        catch (IOException e) {
            throw new ScriptGenException(e);
        }

        // help the garbage collector reclaim things
        root = null;
        doc = null;

        return caseCount;
    }

    public void newTest(int testID) {
        testCase = new TestCase(testID);
        ++caseCount;
    }

    public int endTest() {
        testCase.assemble();
        root.appendChild(testCase.element);
        testCase = null;
        return caseCount;
    }

    private Element newElement(String tag) {
        return doc.createElement(tag);
    }

    private Element newElement(String tag, String attribute, String value) {
        Element element = newElement(tag);
        element.setAttribute(attribute, value);
        return element;
    }

    private Node newParameter(String text) {
        return newElement("parameter", "text", text);
    }

    private Node newVariableAssign(String name, String value,
                                   boolean unsetFirst) {
        Element assignment = newElement("assign");
        assignment.setAttribute("name", name);
        assignment.setAttribute("value", value);
        if (unsetFirst) {
            assignment.setAttribute("unset-first", "true");
        }
        return assignment;
    }

    private Element newFilename(String tag, String filename) {
        return newElement(tag, "filename", filename);
    }

    private Node newExternalScript(String filename, List parameters) {
        Node script = newFilename("external", filename);
        int size = parameters.size();
        Iterator iterator = parameters.iterator();
        for (int i = size; i-- > 0; )
            script.appendChild(newParameter((String) iterator.next()));
        return script;
    }

    private Node newInlineScript(String filename, Map assigns) {
        Node script = newFilename("inline", filename);
        int size = assigns.size();
        Iterator keys = assigns.keySet().iterator();
        for (int i = size; i-- > 0; ) {
            String key = (String) keys.next();
            Node node = newVariableAssign(key, (String) assigns.get(key),
                                          false);
            script.appendChild(node);
        }
        return script;
    }

    public int addParameter(String text) {
        Node parameter = newParameter(text);
        testCase.parameters.add(parameter);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addMoveFile(String source, String destination) {
        Element move = newElement("move");
        move.setAttribute("source", source);
        move.setAttribute("destination", destination);
        if (testCase.moves == null) {
            testCase.moves = new ArrayList();
        }
        testCase.moves.add(move);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addComment(String text) {
        Node comment = doc.createComment(text);
        if (testCase.comments == null) {
            testCase.comments = new ArrayList();
        }
        testCase.comments.add(comment);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addInputFile(String filename) {
        assert testCase.outputFile == null;
        if (filename != "") {
            testCase.inputFile = newFilename("input", filename);
        }
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addOutputFile(String filename) {
        assert testCase.outputFile == null;
        if (filename != "") {
            testCase.outputFile = newFilename("output", filename);
        }
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addExternalSetupScript(String filename, List params) {
        Node script = newExternalScript(filename, params);
        if (testCase.externalSetup == null) {
            testCase.externalSetup = new ArrayList();
        }
        testCase.externalSetup.add(script);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addExternalFinishScript(String filename, List params) {
        Node script = newExternalScript(filename, params);
        if (testCase.externalFinish == null) {
            testCase.externalFinish = new ArrayList();
        }
        testCase.externalFinish.add(script);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int setRunInBackground(boolean flagged) {
        testCase.background = flagged;
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public void addVariableAssign(String name, String value,
                                  boolean unsetFirst) {
        Node assign = newVariableAssign(name, value, unsetFirst);
        root.appendChild(assign);
    }

    public int addInlineSetupScript(String scriptName, Map params) {
        Node script = newInlineScript(scriptName, params);
        if (testCase.inlineSetup == null) {
            testCase.inlineSetup = new ArrayList();
        }
        testCase.inlineSetup.add(script);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addInlineFinishScript(String scriptName, Map params) {
        Node script = newInlineScript(scriptName, params);
        if (testCase.inlineFinish == null) {
            testCase.inlineFinish = new ArrayList();
        }
        testCase.inlineFinish.add(script);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public int addTestDriver(String command) {
        assert testCase.driver == null;
        testCase.driver = newElement("driver", "command", command);
        ++testCase.paramCount;
        return testCase.paramCount;
    }

    public void addVerbatim(String text) {
        root.appendChild(newElement("verbatim", "text", text));
    }

    public String getEscapedQuote() {
        return "\"";
    }
}
